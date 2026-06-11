from flask import Flask, render_template, request, redirect
import subprocess
import json
import os
import uuid
from datetime import datetime

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(BASE_DIR)
PROBLEMS_DIR = os.path.join(PROJECT_DIR, "problems")
MINIJUDGE_PATH = os.path.join(PROJECT_DIR, "minijudge")
SUBMISSIONS_DIR = os.path.join(BASE_DIR, "submissions")

DEFAULT_USER = "guest"


def load_problem(problem_id):
    problem_dir = os.path.join(PROBLEMS_DIR, problem_id)
    problem_json = os.path.join(problem_dir, "problem.json")

    if not os.path.exists(problem_json):
        return None

    with open(problem_json, "r", encoding="utf-8") as f:
        problem = json.load(f)

    problem["id"] = problem_id
    problem["path"] = problem_dir
    return problem


def load_problem_list():
    problems = []

    if not os.path.exists(PROBLEMS_DIR):
        return problems

    for name in sorted(os.listdir(PROBLEMS_DIR)):
        problem_dir = os.path.join(PROBLEMS_DIR, name)
        problem_json = os.path.join(problem_dir, "problem.json")

        if not os.path.isdir(problem_dir):
            continue

        if not os.path.exists(problem_json):
            continue

        with open(problem_json, "r", encoding="utf-8") as f:
            problem = json.load(f)

        problem["id"] = name
        problems.append(problem)

    return problems


def create_submission_file(problem_id, code):
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    random_id = uuid.uuid4().hex[:8]
    submission_id = f"{timestamp}_{random_id}"

    submission_dir = os.path.join(
        SUBMISSIONS_DIR,
        problem_id,
        DEFAULT_USER,
        submission_id
    )

    os.makedirs(submission_dir, exist_ok=True)

    source_path = os.path.join(submission_dir, "main.cpp")

    with open(source_path, "w", encoding="utf-8") as f:
        f.write(code)

    return submission_id, source_path


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


@app.route("/", methods=["GET"])
def home():
    return redirect("/problems")


@app.route("/problems", methods=["GET"])
def problems_page():
    problems = load_problem_list()
    return render_template("problems.html", problems=problems)


@app.route("/problem/<problem_id>", methods=["GET"])
def problem_page(problem_id):
    problem = load_problem(problem_id)

    if problem is None:
        return "题目不存在", 404

    return render_template(
        "index.html",
        problem=problem,
        message="",
        code="",
        status="",
        submission_id="",
        user=DEFAULT_USER
    )


@app.route("/submit/<problem_id>", methods=["POST"])
def submit(problem_id):
    problem = load_problem(problem_id)

    if problem is None:
        return "题目不存在", 404

    code = request.form.get("code", "")

    if code.strip() == "":
        return render_template(
            "index.html",
            problem=problem,
            message="代码为空！",
            code=code,
            status="EMPTY",
            submission_id="",
            user=DEFAULT_USER
        )

    submission_id, source_path = create_submission_file(problem_id, code)

    try:
        result = subprocess.run(
            [MINIJUDGE_PATH, source_path, problem["path"]],
            cwd=PROJECT_DIR,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=10,
            text=True
        )

        output = result.stdout

    except subprocess.TimeoutExpired:
        output = "Flask 调用判题器超时"
    except Exception as e:
        output = f"判题失败：{e}"

    status = parse_status(output)

    message = (
        f"Problem ID: {problem_id}\n"
        f"User: {DEFAULT_USER}\n"
        f"Submission ID: {submission_id}\n\n"
        f"{output}"
    )

    return render_template(
        "index.html",
        problem=problem,
        message=message,
        code=code,
        status=status,
        submission_id=submission_id,
        user=DEFAULT_USER
    )


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=9000, debug=True)
