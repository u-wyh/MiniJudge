import os
import json


def load_problem(problem_id, problems_dir):
    problem_dir = os.path.join(problems_dir, problem_id)
    problem_json = os.path.join(problem_dir, "problem.json")

    if not os.path.exists(problem_json):
        return None

    with open(problem_json, "r", encoding="utf-8") as f:
        problem = json.load(f)

    problem["id"] = problem_id
    problem["path"] = problem_dir
    return problem


def get_problem_stats(problem_id, submissions_dir):
    total = 0
    ac = 0

    problem_submission_dir = os.path.join(submissions_dir, problem_id)

    if not os.path.exists(problem_submission_dir):
        return {
            "total": 0,
            "ac": 0,
            "ac_rate": "0.0%"
        }

    for user in os.listdir(problem_submission_dir):
        user_dir = os.path.join(problem_submission_dir, user)

        if not os.path.isdir(user_dir):
            continue

        for submission_id in os.listdir(user_dir):
            result_path = os.path.join(
                user_dir,
                submission_id,
                "result.txt"
            )

            if not os.path.exists(result_path):
                continue

            total += 1

            with open(result_path, "r", encoding="utf-8") as f:
                for line in f:
                    if line.startswith("Status:"):
                        parts = line.strip().split()
                        if len(parts) >= 2 and parts[1] == "AC":
                            ac += 1
                        break

    ac_rate = "0.0%"
    if total > 0:
        ac_rate = f"{ac * 100.0 / total:.1f}%"

    return {
        "total": total,
        "ac": ac,
        "ac_rate": ac_rate
    }


def load_problem_list(problems_dir, submissions_dir):
    problems = []

    if not os.path.exists(problems_dir):
        return problems

    for name in sorted(os.listdir(problems_dir)):
        problem_dir = os.path.join(problems_dir, name)
        problem_json = os.path.join(problem_dir, "problem.json")

        if not os.path.isdir(problem_dir):
            continue

        if not os.path.exists(problem_json):
            continue

        with open(problem_json, "r", encoding="utf-8") as f:
            problem = json.load(f)

        problem["id"] = name
        problem["stats"] = get_problem_stats(name, submissions_dir)
        problems.append(problem)

    return problems
