#include <gsl_math.h>
#include <gsl_min.h>
#include <gsl_errno.h>
#include <gsl_test.h>

#include "test.h"

/* stopping parameters */

const double EPSABS = 0.001 ;
const double EPSREL = 0.001 ;

const unsigned int MAX_ITERATIONS = 100;

void my_error_handler (const char *reason, const char *file,
		       int line, int err);

#define WITHIN_TOL(a, b, epsrel, epsabs) \
 (fabs((a) - (b)) < (epsrel) * GSL_MIN(fabs(a), fabs(b)) + (epsabs))

int
main (void)
{
  gsl_function F_cos, F_func1, F_func2, F_func3;
  
  const gsl_min_fminimizer_type * fminimizer[4] ;
  const gsl_min_fminimizer_type ** T;

  fminimizer[0] = gsl_min_fminimizer_goldensection;
  fminimizer[1] = gsl_min_fminimizer_brent;
  fminimizer[2] = 0;

  F_cos = create_function (cos) ;
  F_func1 = create_function (func1) ;
  F_func2 = create_function (func2) ;
  F_func3 = create_function (func3) ;

  gsl_set_error_handler (&my_error_handler);

  for (T = fminimizer ; *T != 0 ; T++)
    {
      test_f (*T, "cos(x) [0 (3) 6]", &F_cos, 0.0, 3.0, 6.0, M_PI);
      test_f (*T, "x^4 - 1 [-3 (-1) 17]", &F_func1, -3.0, -1.0, 17.0, 0.0);
      test_f (*T, "sqrt(|x|) [-2 (-1) 1.5]", &F_func2, -2.0, -1.0, 1.5, 0.0);
      test_f (*T, "func3(x) [-2 (3) 4]", &F_func3, -2.0, 3.0, 4.0, 1.0);

      test_f_e (*T, "invalid range check [4, 0]", &F_cos, 4.0, 3.0, 0.0, M_PI);
      test_f_e (*T, "invalid range check [1, 1]", &F_cos, 1.0, 1.0, 1.0, M_PI);
      test_f_e (*T, "invalid range check [-1, 1]", &F_cos, -1.0, 0.0, 1.0, M_PI);
    }

  return gsl_test_summary ();
}

void
test_f (const gsl_min_fminimizer_type * T, 
        const char * description, gsl_function *f,
	double lower_bound, double middle, double upper_bound, 
        double correct_minimum)
{
  int status;
  size_t iterations = 0;
  double m, a, b;
  gsl_interval x;
  gsl_min_fminimizer * s;

  x.lower = lower_bound;
  x.upper = upper_bound;

  s = gsl_min_fminimizer_alloc(T, f, middle, x) ;
  
  do 
    {
      iterations++ ;

      status = gsl_min_fminimizer_iterate (s);

      m = gsl_min_fminimizer_minimum(s);
      x = gsl_min_fminimizer_interval(s);

      a = x.lower;
      b = x.upper;

#ifdef DEBUG
      printf("%.12f %.18f %.12f %.18f %.12f %.18f\n", 
             a, GSL_FN_EVAL(f, a), m, GSL_FN_EVAL(f, m), b, GSL_FN_EVAL(f, b));
#endif

      if (a > b)
	gsl_test (GSL_FAILURE, "interval is invalid (%g,%g)", a, b);

      if (m < a || m > b)
	gsl_test (GSL_FAILURE, "m lies outside interval %g (%g,%g)", m, a, b);

      if (status) break ;

      status = gsl_min_test_interval (x, EPSABS, EPSREL);
    }
  while (status == GSL_CONTINUE && iterations < MAX_ITERATIONS);

  gsl_test (status, "%s, %s (%g obs vs %g expected) ", 
	    gsl_min_fminimizer_name(s), description, 
	    gsl_min_fminimizer_minimum(s), correct_minimum);

  /* check the validity of the returned result */

  if (!WITHIN_TOL (m, correct_minimum, EPSREL, EPSABS))
    {
      gsl_test (GSL_FAILURE, "incorrect precision (%g obs vs %g expected)", 
		m, correct_minimum);
    }
}

void
test_f_e (const gsl_min_fminimizer_type * T, 
	  const char * description, gsl_function *f,
	  double lower_bound, double middle, double upper_bound, 
          double correct_minimum)
{
  int status;
  size_t iterations = 0;
  gsl_interval x;
  gsl_min_fminimizer * s;

  x.lower = lower_bound;
  x.upper = upper_bound;

  s = gsl_min_fminimizer_alloc(T, f, middle, x) ;

  if (s == 0) 
    {
      gsl_test (s != 0, "%s, %s", T->name, description);
      return ;
    }

  do 
    {
      iterations++ ;
      gsl_min_fminimizer_iterate (s);
      status = gsl_min_test_interval (gsl_min_fminimizer_interval(s), 
				      EPSABS, EPSREL);
    }
  while (status == GSL_CONTINUE && iterations < MAX_ITERATIONS);

  gsl_test (!status, "%s, %s", gsl_min_fminimizer_name(s), description, 
	    gsl_min_fminimizer_minimum(s) - correct_minimum);

}

void
my_error_handler (const char *reason, const char *file, int line, int err)
{
  if (0)
    printf ("(caught [%s:%d: %s (%d)])\n", file, line, reason, err);
}
