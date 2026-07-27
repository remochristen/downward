#include "gurobi_solver_interface.h"

#include <stdexcept>
#include <iostream>

namespace lp {

GurobiSolverInterface::GurobiSolverInterface(){
  std::cout << "hello from gurobi\n";
}

void GurobiSolverInterface::load_problem(const LinearProgram &) {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::add_temporary_constraints(
    const named_vector::NamedVector<LPConstraint> &) {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::clear_temporary_constraints() {
    throw std::runtime_error("Not implemented.");
}

double GurobiSolverInterface::get_infinity() const {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::set_objective_coefficients(
    const std::vector<double> &) {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::set_objective_coefficient(int, double) {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::set_constraint_rhs(int, double) {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::set_constraint_sense(int, lp::Sense) {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::set_variable_lower_bound(int, double) {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::set_variable_upper_bound(int, double) {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::set_mip_gap(double) {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::solve() {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::write_lp(const std::string &) const {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::print_failure_analysis() const {
    throw std::runtime_error("Not implemented.");
}

bool GurobiSolverInterface::is_infeasible() const {
    throw std::runtime_error("Not implemented.");
}

bool GurobiSolverInterface::is_unbounded() const {
    throw std::runtime_error("Not implemented.");
}

bool GurobiSolverInterface::has_optimal_solution() const {
    throw std::runtime_error("Not implemented.");
}

double GurobiSolverInterface::get_objective_value() const {
    throw std::runtime_error("Not implemented.");
}

std::vector<double> GurobiSolverInterface::extract_solution() const {
    throw std::runtime_error("Not implemented.");
}

int GurobiSolverInterface::get_num_variables() const {
    throw std::runtime_error("Not implemented.");
}

int GurobiSolverInterface::get_num_constraints() const {
    throw std::runtime_error("Not implemented.");
}

bool GurobiSolverInterface::has_temporary_constraints() const {
    throw std::runtime_error("Not implemented.");
}

void GurobiSolverInterface::print_statistics() const {
    throw std::runtime_error("Not implemented.");
}

}
