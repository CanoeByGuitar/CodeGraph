import subprocess


def run_cpp_program(filename: str = "codegraph"):
    try:
        subprocess.run(["./bin/main", "file", filename])
    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    run_cpp_program("codegraph")
