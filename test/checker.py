import os
import sys
import time
import select
from subprocess import PIPE, Popen


class CustomError(Exception):
    def __init__(self, message: str):
        self.message = f"\033[91;1;4m{message}\033[0m"
        super().__init__(self.message)


PROMPTS = (">> ", ">>", "» ", "»", "$ ", "$", "# ", "#")


def strip_prompt_prefix(line: str) -> str:
    line = line.lstrip()

    changed = True
    while changed:
        changed = False
        for prompt in sorted(PROMPTS, key=len, reverse=True):
            if line.startswith(prompt):
                line = line[len(prompt):].lstrip()
                changed = True
                break

    return line


def normalize_line(line: str) -> str:
    return strip_prompt_prefix(line.rstrip("\n\r")).rstrip()


def resolve_executable(executable: str) -> str:
    if os.path.isabs(executable):
        return executable
    return os.path.abspath(executable)


def consume_prompt_from_buffer(process) -> bool:
    buffer = getattr(process, "_checker_buffer", "")

    for prompt in sorted(PROMPTS, key=len, reverse=True):
        if buffer.startswith(prompt):
            process._checker_buffer = buffer[len(prompt):]
            return True

    return False


def try_read_optional_prompt(process, timeout: float = 0.02) -> str:
    """
    Consume an optional prompt either from the internal buffer or stdout.
    This prevents prompts already read by buffered_readline() from leaking into
    the next output line.
    """
    if consume_prompt_from_buffer(process):
        return "prompt"

    fd = process.stdout.fileno()
    ready, _, _ = select.select([fd], [], [], timeout)

    if not ready:
        return ""

    data = os.read(fd, 16)
    if not data:
        return ""

    text = data.decode("utf-8", errors="replace")
    process._checker_buffer = getattr(process, "_checker_buffer", "") + text

    if consume_prompt_from_buffer(process):
        return "prompt"

    return ""


def buffered_readline(process, timeout: float = 5.0) -> str:
    deadline = time.time() + timeout
    buffer = getattr(process, "_checker_buffer", "")
    process._checker_buffer = ""

    while True:
        if "\n" in buffer:
            line, rest = buffer.split("\n", 1)
            process._checker_buffer = rest
            return line + "\n"

        if time.time() > deadline:
            return buffer

        fd = process.stdout.fileno()
        remaining = max(0.0, deadline - time.time())
        ready, _, _ = select.select([fd], [], [], remaining)

        if not ready:
            continue

        chunk = os.read(fd, 1024)
        if not chunk:
            return buffer

        buffer += chunk.decode("utf-8", errors="replace")


def line_diff_error_message(output_file: str, line_number: int, line: str, expected_line: str) -> str:
    return (
        f"\033[91mError at line {line_number} in file '{output_file}':\n"
        f"\toutput is \033[90m'{line}'\033[91m but expected: "
        f"\033[90m'{expected_line}'\033[0m"
    )


def line_diff(line: str, expected_line: str, context: dict) -> bool:
    normalized_line = normalize_line(line)
    normalized_expected = normalize_line(expected_line)

    if normalized_line == normalized_expected:
        return True

    print(
        line_diff_error_message(
            context["output_file"],
            context["line_number"],
            line,
            expected_line,
        ),
        file=sys.stderr,
    )
    return False


def check_output(output_file_name: str, expected_output_file_name: str) -> float:
    with open(output_file_name, "r", encoding="utf-8") as output_file:
        output = output_file.read().splitlines()

    with open(expected_output_file_name, "r", encoding="utf-8") as expected_output_file:
        expected_output = expected_output_file.read().splitlines()

    sum_matches = 0

    for index, expected_line in enumerate(expected_output):
        actual_line = output[index] if index < len(output) else ""

        if line_diff(
            actual_line,
            expected_line,
            {"output_file": output_file_name, "line_number": index + 1},
        ):
            sum_matches += 1

    if len(output) != len(expected_output):
        print(
            f"\033[91mLine count mismatch in '{output_file_name}': "
            f"got {len(output)}, expected {len(expected_output)}\033[0m",
            file=sys.stderr,
        )

    if not expected_output:
        return 1.0 if not output else 0.0

    return sum_matches / len(expected_output)


def run(
    executable: str,
    input_file_name: str,
    output_file_name: str,
    expected_output_file_name: str,
) -> float:
    with open(input_file_name, "r", encoding="utf-8") as input_file, open(
        output_file_name, "w", encoding="utf-8"
    ) as output_file:

        process = Popen(
            ["stdbuf", "-oL", resolve_executable(executable)],
            stdout=PIPE,
            stdin=PIPE,
            text=False,
        )

        process._checker_buffer = ""

        for line_number, raw_line in enumerate(input_file, start=1):
            try_read_optional_prompt(process)

            command = normalize_line(raw_line)

            process.stdin.write((command + "\n").encode("utf-8"))
            process.stdin.flush()

            if command == "Exit":
                break

            output_line = buffered_readline(process, timeout=5.0)

            if output_line == "":
                raise CustomError(
                    f"Process terminated before producing output for line {line_number}"
                )

            output_file.write(normalize_line(output_line) + "\n")

        process.stdin.close()

        # Flush any remaining final output line.
        while True:
            leftover = buffered_readline(process, timeout=0.05)
            if not leftover:
                break
            output_file.write(normalize_line(leftover) + "\n")

        process.stdout.close()
        process.wait(timeout=5)

    return check_output(output_file_name, expected_output_file_name)


if __name__ == "__main__":
    if len(sys.argv) != 5:
        print(
            "Usage: python3 checker.py "
            "<executable> <input_file> <output_file> <expected_output_file>"
        )
        sys.exit(1)

    grade = run(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    print(grade)