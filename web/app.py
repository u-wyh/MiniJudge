from flask import Flask, render_template, request
import os

from services.problem_service import load_problem, load_problem_list
from services.submission_service import (
    create_submission_file,
    parse_status,
    run_judge,
    save_result,
    cleanup_old_submissions,
    load_submissions,
    load_submission_detail
)

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(BASE_DIR)
PROBLEMS_DIR = os.path.join(PROJECT_DIR, "problems")
MINIJUDGE_PATH = os.path.join(PROJECT_DIR, "minijudge")
SUBMISSIONS_DIR = os.path.join(BASE_DIR, "submissions")

DEFAULT_USER = "guest"
MAX_SUBMISSIONS_PER_USER_PROBLEM = 50

SUPPORTED_LANGUAGES = [
    {
        "id": "cpp17",
        "name": "C++17"
    }
]


def get_language_name(language_id):
    for lang in SUPPORTED_LANGUAGES:
        if lang["id"] == language_id:
            return lang["name"]
    return "C++17"


def get_dashboard_stats():
    problems = load_problem_list(PROBLEMS_DIR, SUBMISSIONS_DIR)
    submissions = load_submissions(SUBMISSIONS_DIR)

    total_problems = len(problems)
    total_submissions = len(submissions)
    total_ac = sum(1 for s in submissions if s["status"] == "AC")

    ac_rate = "0.0%"
    if total_submissions > 0:
        ac_rate = f"{total_ac * 100.0 / total_submissions:.1f}%"

    return {
        "total_problems": total_problems,
        "total_submissions": total_submissions,
        "total_ac": total_ac,
        "ac_rate": ac_rate
    }


def get_submission_stats(submissions):
    total = len(submissions)

    stats = {
        "total": total,
        "AC": 0,
        "WA": 0,
        "CE": 0,
        "RE": 0,
        "TLE": 0,
        "UNKNOWN": 0,
        "ac_rate": "0.0%"
    }

    for s in submissions:
        status = s["status"]
        if status in stats:
            stats[status] += 1
        else:
            stats["UNKNOWN"] += 1

    if total > 0:
        stats["ac_rate"] = f"{stats['AC'] * 100.0 / total:.1f}%"

    return stats


@app.route("/", methods=["GET"])
def home():
    submissions = load_submissions(SUBMISSIONS_DIR)

    return render_template(
        "dashboard.html",
        stats=get_dashboard_stats(),
        recent_submissions=submissions[:5]
    )


@app.route("/problems", methods=["GET"])
def problems_page():
    selected_sort = request.args.get("sort", "id")
    keyword = request.args.get("keyword", "").strip()

    problems = load_problem_list(PROBLEMS_DIR, SUBMISSIONS_DIR)

    if keyword:
        lower_keyword = keyword.lower()
        problems = [
            p for p in problems
            if lower_keyword in p["id"].lower()
            or lower_keyword in p["title"].lower()
        ]

    def rate_value(problem):
        rate = problem["stats"]["ac_rate"]
        try:
            return float(rate.replace("%", ""))
        except ValueError:
            return 0.0

    if selected_sort == "total_desc":
        problems.sort(
            key=lambda p: (p["stats"]["total"], p["id"]),
            reverse=True
        )
    elif selected_sort == "ac_desc":
        problems.sort(
            key=lambda p: (p["stats"]["ac"], p["id"]),
            reverse=True
        )
    elif selected_sort == "rate_desc":
        problems.sort(
            key=lambda p: (rate_value(p), p["id"]),
            reverse=True
        )
    else:
        problems.sort(key=lambda p: p["id"])

    return render_template(
        "problems.html",
        problems=problems,
        selected_sort=selected_sort,
        keyword=keyword
    )


@app.route("/submissions", methods=["GET"])
def submissions_page():
    selected_problem_id = request.args.get("problem_id", "")
    selected_status = request.args.get("status", "")
    selected_sort = request.args.get("sort", "time_desc")
    keyword = request.args.get("keyword", "").strip()

    try:
        page = int(request.args.get("page", "1"))
    except ValueError:
        page = 1

    if page < 1:
        page = 1

    per_page = 20

    submissions = load_submissions(SUBMISSIONS_DIR)
    problems = load_problem_list(PROBLEMS_DIR, SUBMISSIONS_DIR)

    if selected_problem_id:
        submissions = [
            s for s in submissions
            if s["problem_id"] == selected_problem_id
        ]

    if selected_status:
        submissions = [
            s for s in submissions
            if s["status"] == selected_status
        ]

    if keyword:
        lower_keyword = keyword.lower()
        submissions = [
            s for s in submissions
            if lower_keyword in s["submission_id"].lower()
            or lower_keyword in s["problem_id"].lower()
            or lower_keyword in s["user"].lower()
            or lower_keyword in s.get("language", "").lower()
            or lower_keyword in s["status"].lower()
            or lower_keyword in s["created_at"].lower()
        ]

    if selected_sort == "time_asc":
        submissions.sort(key=lambda x: x["submission_id"])
    elif selected_sort == "problem_id":
        submissions.sort(
            key=lambda x: (x["problem_id"], x["submission_id"]),
            reverse=True
        )
    elif selected_sort == "status":
        submissions.sort(
            key=lambda x: (x["status"], x["submission_id"]),
            reverse=True
        )
    else:
        submissions.sort(key=lambda x: x["submission_id"], reverse=True)

    stats = get_submission_stats(submissions)

    total = len(submissions)
    total_pages = (total + per_page - 1) // per_page

    if total_pages == 0:
        total_pages = 1

    if page > total_pages:
        page = total_pages

    start = (page - 1) * per_page
    end = start + per_page
    page_submissions = submissions[start:end]

    return render_template(
        "submissions.html",
        submissions=page_submissions,
        problems=problems,
        selected_problem_id=selected_problem_id,
        selected_status=selected_status,
        selected_sort=selected_sort,
        keyword=keyword,
        page=page,
        total_pages=total_pages,
        total=total,
        stats=stats
    )


@app.route("/problem/<problem_id>", methods=["GET"])
def problem_page(problem_id):
    problem = load_problem(problem_id, PROBLEMS_DIR)

    if problem is None:
        return render_template(
            "error.html",
            title="404 - 题目不存在",
            message="你访问的题目不存在，可能是题号错误或题目文件缺失。"
        ), 404

    all_submissions = load_submissions(SUBMISSIONS_DIR)
    recent_submissions = [
        s for s in all_submissions
        if s["problem_id"] == problem_id
    ][:5]

    return render_template(
        "index.html",
        problem=problem,
        message="",
        code="",
        status="",
        submission_id="",
        user=DEFAULT_USER,
        languages=SUPPORTED_LANGUAGES,
        selected_language="cpp17",
        recent_submissions=recent_submissions
    )


@app.route("/submit/<problem_id>", methods=["POST"])
def submit(problem_id):
    problem = load_problem(problem_id, PROBLEMS_DIR)

    if problem is None:
        return render_template(
            "error.html",
            title="404 - 题目不存在",
            message="你访问的题目不存在，可能是题号错误或题目文件缺失。"
        ), 404

    language = request.form.get("language", "cpp17")
    code = request.form.get("code", "")

    if code.strip() == "":
        return render_template(
            "index.html",
            problem=problem,
            message="代码为空！",
            code=code,
            status="EMPTY",
            submission_id="",
            user=DEFAULT_USER,
            languages=SUPPORTED_LANGUAGES,
            selected_language=language
        )

    submission_id, source_path, submission_dir = create_submission_file(
        SUBMISSIONS_DIR,
        problem_id,
        DEFAULT_USER,
        code
    )

    try:
        output = run_judge(
            MINIJUDGE_PATH,
            source_path,
            problem["path"],
            PROJECT_DIR
        )
    except Exception as e:
        output = f"Result: WEB_ERROR\n判题器调用失败：{e}\n"

    status = parse_status(output)

    language_name = get_language_name(language)

    save_result(
        submission_dir,
        problem_id,
        DEFAULT_USER,
        submission_id,
        status,
        language_name,
        output
    )

    cleanup_old_submissions(
        SUBMISSIONS_DIR,
        problem_id,
        DEFAULT_USER,
        MAX_SUBMISSIONS_PER_USER_PROBLEM
    )

    message = (
        f"Problem ID: {problem_id}\n"
        f"User: {DEFAULT_USER}\n"
        f"Submission ID: {submission_id}\n"
        f"Language: {language_name}\n\n"
        f"{output}"
    )

    return render_template(
        "index.html",
        problem=problem,
        message=message,
        code=code,
        status=status,
        submission_id=submission_id,
        user=DEFAULT_USER,
        languages=SUPPORTED_LANGUAGES,
        selected_language=language
    )


@app.route("/submission/<problem_id>/<user>/<submission_id>", methods=["GET"])
def submission_detail(problem_id, user, submission_id):
    detail = load_submission_detail(
        SUBMISSIONS_DIR,
        problem_id,
        user,
        submission_id
    )

    if detail is None:
        return render_template(
            "error.html",
            title="404 - 提交不存在",
            message="你访问的提交记录不存在，可能已被清理或提交 ID 错误。"
        ), 404

    return render_template(
        "submission_detail.html",
        problem_id=detail["problem_id"],
        user=detail["user"],
        submission_id=detail["submission_id"],
        status=detail["status"],
        language=detail["language"],
        source_code=detail["source_code"],
        result_text=detail["result_text"]
    )


@app.errorhandler(404)
def not_found(error):
    return render_template(
        "error.html",
        title="404 - 页面不存在",
        message="你访问的页面不存在，请检查地址是否正确。"
    ), 404


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=9000, debug=True)
