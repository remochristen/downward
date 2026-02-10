#include "highs_solver_interface.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace lp {

static void highs_ok(HighsStatus s, const char* where) {
    if (s == HighsStatus::kOk) return;
    throw std::runtime_error(std::string("HiGHS error in ") + where);
}


HiGHSSolverInterface::HiGHSSolverInterface()
    : highs_(),
      num_permanent_constraints(0),
      num_temporary_constraints(0) {
    highs_ok(highs_.setOptionValue("output_flag", false), "setOptionValue(output_flag)");
    highs_ok(highs_.setOptionValue("log_to_console", false), "setOptionValue(log_to_console)");
}

void HiGHSSolverInterface::load_problem(const LinearProgram &lp) {
    highs_ok(highs_.clear(), "clear");

    highs_ok(highs_.setOptionValue("output_flag", false), "setOptionValue(output_flag)");
    highs_ok(highs_.setOptionValue("log_to_console", false), "setOptionValue(log_to_console)");

    if (lp.get_sense() == LPObjectiveSense::MAXIMIZE) {
        highs_ok(highs_.changeObjectiveSense(ObjSense::kMaximize), "changeObjectiveSense(max)");
    } else {
        highs_ok(highs_.changeObjectiveSense(ObjSense::kMinimize), "changeObjectiveSense(min)");
    }

    const double inf = highs_.getInfinity();

    // Variables
    const auto &vars = lp.get_variables();
    const int n = static_cast<int>(vars.size());
    for (int i = 0; i < n; ++i) {
        const auto &v = vars[i];

        double lb = 0.0;
        double ub = 0.0;
        sense_rhs_to_bounds(v.sense, v.right_hand_side, inf, lb, ub);

        highs_ok(highs_.addCol(
                     v.objective_coefficient,
                     lb, ub,
                     0, nullptr, nullptr),
                 "addCol");

        if (v.is_integer) {
            highs_ok(highs_.changeColIntegrality(i, HighsVarType::kInteger),
                     "changeColIntegrality");
        }
    }

    // Constraints
    const auto &cons = lp.get_constraints();
    const int m = static_cast<int>(cons.size());
    for (int r = 0; r < m; ++r) {
        const auto &c = cons[r];
        const auto &idx = c.get_variables();
        const auto &val = c.get_coefficients();
        assert(idx.size() == val.size());

        double row_lb = 0.0;
        double row_ub = 0.0;
        sense_rhs_to_bounds(c.get_sense(), c.get_right_hand_side(), inf, row_lb, row_ub);

        highs_ok(highs_.addRow(
                     row_lb,
                     row_ub,
                     static_cast<int>(idx.size()),
                     idx.data(),
                     val.data()),
                 "addRow");
    }

    num_permanent_constraints = highs_.getNumRow();
    num_temporary_constraints = 0;
}

void HiGHSSolverInterface::add_temporary_constraints(
    const named_vector::NamedVector<LPConstraint> &constraints) {

    if (constraints.empty())
        return;

    const double inf = get_infinity();

    for (int i = 0; i < static_cast<int>(constraints.size()); ++i) {
        const auto &c = constraints[i];
        const auto &idx = c.get_variables();
        const auto &val = c.get_coefficients();
        assert(idx.size() == val.size());

        double row_lb = 0.0;
        double row_ub = 0.0;
        sense_rhs_to_bounds(c.get_sense(), c.get_right_hand_side(), inf, row_lb, row_ub);

        highs_ok(highs_.addRow(
                     row_lb,
                     row_ub,
                     static_cast<int>(idx.size()),
                     idx.data(),
                     val.data()),
                 "addRow(temp)");
    }

    num_temporary_constraints += static_cast<int>(constraints.size());
}

void HiGHSSolverInterface::clear_temporary_constraints() {
    if (num_temporary_constraints == 0)
        return;

    const int first_temp_row = num_permanent_constraints;
    const int last_row = highs_.getNumRow() - 1;

    if (first_temp_row <= last_row) {
        highs_ok(highs_.deleteRows(first_temp_row, last_row), "deleteRows");
    }

    num_temporary_constraints = 0;
}

double HiGHSSolverInterface::get_infinity() const {
    return highs_.getInfinity();
}

void HiGHSSolverInterface::set_objective_coefficients(const std::vector<double> &coefficients) {
    const int n = highs_.getNumCol();
    for (int i = 0; i < n; ++i) {
        highs_ok(highs_.changeColCost(i, coefficients[i]), "changeColCost(vec)");
    }

}

void HiGHSSolverInterface::set_objective_coefficient(int index, double coefficient) {
    highs_ok(highs_.changeColCost(index, coefficient), "changeColCost");
}

void HiGHSSolverInterface::set_constraint_lower_bound(int index, double bound) {
    const double ub = highs_.getLp().row_upper_[index];
    highs_ok(highs_.changeRowBounds(index, bound, ub), "changeRowBounds(row lb)");
}

void HiGHSSolverInterface::set_constraint_upper_bound(int index, double bound) {
    const double lb = highs_.getLp().row_lower_[index];
    highs_ok(highs_.changeRowBounds(index, lb, bound), "changeRowBounds(row ub)");
}

void HiGHSSolverInterface::set_variable_lower_bound(int index, double bound) {
    const double ub = highs_.getLp().col_upper_[index];
    highs_ok(highs_.changeColBounds(index, bound, ub), "changeColBounds(col lb)");
}

void HiGHSSolverInterface::set_variable_upper_bound(int index, double bound) {
    const double lb = highs_.getLp().col_lower_[index];
    highs_ok(highs_.changeColBounds(index, lb, bound), "changeColBounds(col ub)");
}

void HiGHSSolverInterface::set_constraint_bound(int index, Sense sense, double right_hand_side) {
    const double inf = get_infinity();

    switch (sense) {
    case Sense::LE:
        set_constraint_lower_bound(index, -inf);
        set_constraint_upper_bound(index, right_hand_side);
        break;
    case Sense::GE:
        set_constraint_lower_bound(index, right_hand_side);
        set_constraint_upper_bound(index, inf);
        break;
    case Sense::EQ:
        set_constraint_lower_bound(index, right_hand_side);
        set_constraint_upper_bound(index, right_hand_side);
        break;
    default:
        throw std::logic_error("unknown sense");
    }
}

void HiGHSSolverInterface::set_variable_bound(int index, Sense sense, double right_hand_side) {
    const double inf = get_infinity();

    switch (sense) {
    case Sense::LE:
        set_variable_lower_bound(index, -inf);
        set_variable_upper_bound(index, right_hand_side);
        break;
    case Sense::GE:
        set_variable_lower_bound(index, right_hand_side);
        set_variable_upper_bound(index, inf);
        break;
    case Sense::EQ:
        set_variable_lower_bound(index, right_hand_side);
        set_variable_upper_bound(index, right_hand_side);
        break;
    default:
        throw std::logic_error("unknown sense");
    }
}

void HiGHSSolverInterface::set_mip_gap(double gap) {
    highs_ok(highs_.setOptionValue("mip_rel_gap", gap), "setOptionValue(mip_rel_gap)");
}

void HiGHSSolverInterface::solve() {
    highs_ok(highs_.run(), "run");
}

void HiGHSSolverInterface::write_lp(const std::string &filename) const {
    highs_ok(highs_.writeModel(filename), "writeModel");
}

void HiGHSSolverInterface::print_failure_analysis() const {
    std::cout << highs_.modelStatusToString(highs_.getModelStatus()) << "\n";
}

bool HiGHSSolverInterface::is_infeasible() const {
    return highs_.getModelStatus() == HighsModelStatus::kInfeasible;
}

bool HiGHSSolverInterface::is_unbounded() const {
    return highs_.getModelStatus() == HighsModelStatus::kUnbounded;
}

bool HiGHSSolverInterface::has_optimal_solution() const {
    return highs_.getModelStatus() == HighsModelStatus::kOptimal;
}

double HiGHSSolverInterface::get_objective_value() const {
    assert(has_optimal_solution());
    const HighsInfo& info = highs_.getInfo();
    return info.objective_function_value;
}

std::vector<double> HiGHSSolverInterface::extract_solution() const {
    assert(has_optimal_solution());

    const HighsSolution sol = highs_.getSolution();
    const int n = highs_.getNumCol();

    std::vector<double> x(n, 0.0);
    for (int i = 0; i < n; i++) {
        x[i] = sol.col_value[i];
    }
    return x;
}

int HiGHSSolverInterface::get_num_variables() const {
    return highs_.getNumCol();
}

int HiGHSSolverInterface::get_num_constraints() const {
    return highs_.getNumRow();
}

bool HiGHSSolverInterface::has_temporary_constraints() const {
    return num_temporary_constraints != 0;
}

void HiGHSSolverInterface::print_statistics() const {
    std::cout << highs_.modelStatusToString(highs_.getModelStatus()) << "\n";
}

}

