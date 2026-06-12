import os
import uuid
import shutil
import subprocess
from datetime import datetime


def create_submission_file(submissions_dir, problem_id, user, code):
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    random_id = uuid.uuid4().hex[:8]
    submission_id = f"{timestamp}_{random_id}"

    submission_dir = os.path.join(
        submissions_dir,
        problem_id,
        user,
        submission_id
    )

    os.makedirs(submission_dir, exist_ok=True)

    source_path = os.path.join(submission_dir, "main.cpp")

    with open(source_path, "w", encoding="utf-8") as f:
        f.write(code)

    return submission_id, source_path, submission_dir


def parse_status(output):
    for line in output.splitlines():
        line = line.strip()

        if line.startswith("Final:"):
            parts = line.split()
            if len(parts) >= 2:
                return parts[1]

        if line.startswith("Result:"):
            parts = line.split()
            if len(parts) >= 2:
                return parts[1]

    return "UNKNOWN"


def run_judge(minijudge_path, source_path, problem_path, project_dir):
    try:
        result = subprocess.run(
            [minijudge_path, source_path, problem_path],
            cwd=project_dir,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=15,
            text=True
        )

        return result.stdout

    except subprocess.TimeoutExpired:
        return (
            "Result: WEB_TIMEOUT\n"
            "Flask 等待判题器超过 15 秒。\n"
            "这不是用户程序 TLE，而是 Web 后端调用判题器超时。\n"
        )


def save_result(submission_dir, problem_id, user, submission_id, status, language, output):
    result_path = os.path.join(submission_dir, "result.txt")

    with open(result_path, "w", encoding="utf-8") as f:
        f.write(f"Problem ID: {problem_id}\n")
        f.write(f"User: {user}\n")
        f.write(f"Submission ID: {submission_id}\n")
        f.write(f"Status: {status}\n")
        f.write(f"Language: {language}\n\n")
        f.write(output)


def cleanup_old_submissions(submissions_dir, problem_id, user, max_count):
    user_dir = os.path.join(
        submissions_dir,
        problem_id,
        user
    )

    if not os.path.exists(user_dir):
        return

    submission_dirs = []

    for name in os.listdir(user_dir):
        path = os.path.join(user_dir, name)
        if os.path.isdir(path):
            submission_dirs.append(name)

    submission_dirs.sort(reverse=True)

    old_submissions = submission_dirs[max_count:]

    for submission_id in old_submissions:
        old_path = os.path.join(user_dir, submission_id)
        shutil.rmtree(old_path, ignore_errors=True)


def load_submissions(submissions_dir):
    records = []

    if not os.path.exists(submissions_dir):
        return records

    for problem_id in sorted(os.listdir(submissions_dir)):
        problem_dir = os.path.join(submissions_dir, problem_id)
        if not os.path.isdir(problem_dir):
            continue

        for user in sorted(os.listdir(problem_dir)):
            user_dir = os.path.join(problem_dir, user)
            if not os.path.isdir(user_dir):
                continue

            for submission_id in sorted(os.listdir(user_dir), reverse=True):
                submission_dir = os.path.join(user_dir, submission_id)
                result_path = os.path.join(submission_dir, "result.txt")

                if not os.path.exists(result_path):
                    continue

                status = "UNKNOWN"
                language = "C++17"

                with open(result_path, "r", encoding="utf-8") as f:
                    for line in f:
                        if line.startswith("Status:"):
                            parts = line.strip().split(maxsplit=1)
                            if len(parts) >= 2:
                                status = parts[1]
                        elif line.startswith("Language:"):
                            parts = line.strip().split(maxsplit=1)
                            if len(parts) >= 2:
                                language = parts[1]

                raw_time = submission_id[:15]

                try:
                    created_at = datetime.strptime(
                        raw_time,
                        "%Y%m%d_%H%M%S"
                    ).strftime("%Y-%m-%d %H:%M:%S")
                except ValueError:
                    created_at = raw_time.replace("_", " ")

                records.append({
                    "problem_id": problem_id,
                    "user": user,
                    "submission_id": submission_id,
                    "status": status,
                    "language": language,
                    "created_at": created_at
                })

    records.sort(key=lambda x: x["submission_id"], reverse=True)
    return records


def load_submission_detail(submissions_dir, problem_id, user, submission_id):
    submission_dir = os.path.join(
        submissions_dir,
        problem_id,
        user,
        submission_id
    )

    source_path = os.path.join(submission_dir, "main.cpp")
    result_path = os.path.join(submission_dir, "result.txt")

    if not os.path.exists(submission_dir):
        return None

    source_code = ""
    result_text = ""
    status = "UNKNOWN"
    language = "C++17"

    if os.path.exists(source_path):
        with open(source_path, "r", encoding="utf-8") as f:
            source_code = f.read()

    if os.path.exists(result_path):
        with open(result_path, "r", encoding="utf-8") as f:
            result_text = f.read()

        for line in result_text.splitlines():
            if line.startswith("Status:"):
                parts = line.split(maxsplit=1)
                if len(parts) >= 2:
                    status = parts[1]
            elif line.startswith("Language:"):
                parts = line.split(maxsplit=1)
                if len(parts) >= 2:
                    language = parts[1]

    return {
        "problem_id": problem_id,
        "user": user,
        "submission_id": submission_id,
        "status": status,
        "language": language,
        "source_code": source_code,
        "result_text": result_text
    }
