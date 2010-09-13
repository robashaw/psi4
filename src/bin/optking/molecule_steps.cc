/*! \file molecule.cc
    \ingroup OPT10
    \brief molecule class (really, molecular system class)
*/

#include "molecule.h"

#include <cmath>
#include <iostream>
#include <sstream>

#include "linear_algebra.h"
#include "print.h"
#include "atom_data.h"
#include "physconst.h"

#define EXTERN
#include "globals.h"

using psi::outfile;

namespace opt {

// compute change in energy according to quadratic approximation
inline double DE_nr_energy(double step, double grad, double hess) {
  return (step * grad + 0.5 * step * step * hess);
}

// compute change in energy according to RFO approximation
inline double DE_rfo_energy(double rfo_t, double rfo_g, double rfo_h) {
  return (rfo_t * rfo_g + 0.5 * rfo_t * rfo_t * rfo_h)/(1 + rfo_t*rfo_t);
}

void MOLECULE::nr_step(void) {
  int i, f;
  int Nintco = g_nintco();
  double **H_inv;

  double *f_q = p_Opt_data->g_forces_pointer();
  double **H = p_Opt_data->g_H_pointer();
  double *dq = p_Opt_data->g_dq_pointer();

  double *nr_u;      // unit vector in step direction
  double nr_dqnorm;   // norm of step
  double nr_g;         // gradient in step direction
  double nr_h;         // hessian in step direction
  double DE_projected; // projected energy change by quadratic approximation

  // Hinv f_q = dq
  H_inv = symm_matrix_inv(H, Nintco, 1);
  opt_matrix_mult(H_inv, 0, &f_q, 1, &dq, 1, Nintco, Nintco, 1, 0);
  free_matrix(H_inv);

  // applies maximum internal coordinate change
  apply_intrafragment_step_limit(dq);

  // get norm |q| and unit vector in the step direction
  nr_dqnorm = sqrt( array_dot(dq, dq, Nintco) );
  nr_u = init_array(Nintco);
  array_copy(dq, nr_u, Nintco);
  array_normalize(nr_u, Nintco);
  
  // get gradient and hessian in step direction
  nr_g = -1 * array_dot(f_q, nr_u, Nintco); // gradient, not force

  nr_h = 0;
  for (i=0; i<Nintco; ++i)
    nr_h += nr_u[i] * array_dot(H[i], nr_u, Nintco);

  DE_projected = DE_nr_energy(nr_dqnorm, nr_g, nr_h);
  fprintf(outfile,"\tProjected energy change by quadratic approximation: %20.10lf\n", DE_projected);

  // do displacements for each fragment separately
  double *f_frag;
  for (f=0; f<fragments.size(); ++f) {
    f_frag = init_array(fragments[f]->g_nintco());
    for (i=0; i<fragments[f]->g_nintco(); ++i)
      f_frag[i] = f_q[ g_intco_offset(f) + i];
    fragments[f]->displace(dq, true);
    free_array(f_frag);
  }

  // save values in step data
  p_Opt_data->save_step_info(DE_projected, nr_u, nr_dqnorm, nr_g, nr_h);

  free_array(nr_u);
}

// Take Rational Function Optimization step
void MOLECULE::rfo_step(void) {
  int i, j;
  int dim = g_nintco();
  double *f_q = p_Opt_data->g_forces_pointer();
  double **H = p_Opt_data->g_H_pointer();
  double *dq = p_Opt_data->g_dq_pointer();

  // build (lower-triangle of) RFO matrix and diagonalize
  double **rfo_mat = init_matrix(dim+1, dim+1);
  for (i=0; i<dim; ++i)
    for (j=i; j<dim; ++j)
      rfo_mat[i][j] = H[i][j];

  for (i=0; i<dim; ++i)
    rfo_mat[dim][i] = - f_q[i];

  if (Opt_params.print_lvl >= 3) {
    fprintf(outfile,"RFO mat\n");
    print_matrix(outfile, rfo_mat, dim+1, dim+1);
  }

  double *lambda = init_array(dim+1);
  opt_symm_matrix_eig(rfo_mat, dim+1, lambda);
  if (Opt_params.print_lvl >= 2) {
    fprintf(outfile,"RFO eigenvalues/lambdas\n");
    print_matrix(outfile, &(lambda), 1, dim+1);
  }

  // Do intermediate normalization.  How?  Does it matter?
  // RFO paper says to make the last element '1' and then "scaling of the eigenvector
  // by a factor alpha used to calculate mode displacements" (see page 54) amounts to
  // the same thing as just making sure the sign of the last entry of the evect is positive
  //  During the course of an optimization some evects may appear that are bogus leads (presumably)
  // due to a bad hessian - the root following can avoid them.  also some evects have a virtual
  // 0 in the last entry and so cannot be divided by it
  for (i=0; i<dim+1; ++i) {
    if (rfo_mat[i][dim] < 0)
      for (j=0;j<dim+1;++j) rfo_mat[i][j] *= -1;
  }
  if (Opt_params.print_lvl >= 3) {
    fprintf(outfile,"RFO evects\n");
    print_matrix(outfile, rfo_mat, dim+1, dim+1);
  }

  int rfo_root, f;
  double tval, tval2, rfo_eval;
  double *rfo_u;      // unit vector in step direction
  double rfo_dqnorm;   // norm of step
  double rfo_g;         // gradient in step direction
  double rfo_h;         // hessian in step direction
  double DE_projected; // projected energy change by quadratic approximation

  // *** choose which RFO eigenvector to use
  // if not root following, then use rfo_root'th lowest eigenvalue; default is 0 (lowest)
  if ( (!Opt_params.rfo_follow_root) || (p_Opt_data->g_iteration() == 1)) {
    rfo_root = Opt_params.rfo_root;
    fprintf(outfile,"Following RFO solution %d.\n", rfo_root+1);
  }
  else { // do root following
    double * rfo_old_evect = p_Opt_data->g_rfo_eigenvector_pointer();
    tval = 0;
    for (i=0; i<dim+1; ++i) {
      tval2 = array_dot(rfo_mat[i], rfo_old_evect,dim+1);
      if (tval2 > tval) {
        tval = tval2;
        rfo_root = i;
      }
    }
    fprintf(outfile,"RFO vector %d has maximal overlap with previous step\n", rfo_root+1);
  }
  p_Opt_data->set_rfo_eigenvector(rfo_mat[rfo_root]);

  // print out lowest energy evects
  if (Opt_params.print_lvl >= 2) {
    for (i=0; i<dim+1; ++i) {
      if ((lambda[i] < 0.0) || (i <rfo_root)) {
        fprintf(outfile,"RFO eigenvalue %d: %15.10lf (or 2*%-15.10lf)\n", i+1, lambda[i],lambda[i]/2);
        fprintf(outfile,"eigenvector:\n");
        print_matrix(outfile, &(rfo_mat[i]), 1, dim+1);
      }
    }
  }
  free_array(lambda);

  for (j=0; j<dim; ++j)
    dq[j] = rfo_mat[rfo_root][j]; // leave out last column

  apply_intrafragment_step_limit(dq);
  //check_intrafragment_zero_angles(dq);

  // get norm |dq| and unit vector in the step direction
  rfo_dqnorm = sqrt( array_dot(dq, dq, dim) );
  rfo_u = init_array(dim);
  array_copy(rfo_mat[rfo_root], rfo_u, dim);
  array_normalize(rfo_u, dim);
  free_matrix(rfo_mat);

  // get gradient and hessian in step direction
  rfo_g = -1 * array_dot(f_q, rfo_u, dim);
  rfo_h = 0;
  for (i=0; i<dim; ++i)
    rfo_h += rfo_u[i] * array_dot(H[i], rfo_u, dim);

  DE_projected = DE_rfo_energy(rfo_dqnorm, rfo_g, rfo_h);
  fprintf(outfile,"\tProjected energy change by RFO approximation: %20.10lf\n", DE_projected);

  // do displacements for each fragment separately
//  double *f_frag;
  for (f=0; f<fragments.size(); ++f) {
    //f_frag = init_array(fragments[f]->g_nintco());
    //for (i=0; i<fragments[f]->g_nintco(); ++i)
      //f_frag[i] = f_q[ g_intco_offset(f) + i];
    fragments[f]->displace(&(dq[g_intco_offset(f)]), true);
    //free_array(f_frag);
  }

  // save values in step data
  p_Opt_data->save_step_info(DE_projected, rfo_u, rfo_dqnorm, rfo_g, rfo_h);

  free_array(rfo_u);

} // end take RFO step

}
