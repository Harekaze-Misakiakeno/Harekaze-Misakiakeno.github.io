/*-----------------------------------------------------------------------------
 * Some sample code for slvs.dll. We draw some geometric entities, provide
 * initial guesses for their positions, and then constrain them. The solver
 * calculates their new positions, in order to satisfy the constraints.
 *
 * Copyright -2008-2013 Jonathan Westhues.
 *---------------------------------------------------------------------------*/
#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "slvs.h"

static Slvs_System sys;

static void *CheckMalloc(size_t n)
{
  void *r = malloc(n);
  if (!r)
  {
    printf("out of memory!\n");
    exit(-1);
  }
  return r;
}

/*-----------------------------------------------------------------------------
 * An example of a constraint in 3d. We create a single group, with some
 * entities and constraints.
 *---------------------------------------------------------------------------*/

int solver(int nPts, float *p_ptr, int nConst, float *c_ptr, int nLinks, float *l_ptr, int geomStartIdx)
{
  // printf("first %i \n", (int)*(l_ptr + 1));
  Slvs_hGroup g;
  double qw, qx, qy, qz;

  g = 1;
  /* First, we create our workplane. Its origin corresponds to the origin
     * of our base frame (x y z) = (0 0 0) */
  sys.param[sys.params++] = Slvs_MakeParam(1, g, 0.0);
  sys.param[sys.params++] = Slvs_MakeParam(2, g, 1.0);
  sys.param[sys.params++] = Slvs_MakeParam(3, g, -1.0);
  sys.entity[sys.entities++] = Slvs_MakePoint3d(-101, g, 1, 1, 1);
  /* and it is parallel to the xy plane, so it has basis vectors (1 0 0)
     * and (0 1 0).  */
  Slvs_MakeQuaternion(1, 0, 0,
                      0, 1, 0, &qw, &qx, &qy, &qz);
  sys.param[sys.params++] = Slvs_MakeParam(4, g, qw);
  sys.param[sys.params++] = Slvs_MakeParam(5, g, qx);
  sys.param[sys.params++] = Slvs_MakeParam(6, g, qy);
  sys.param[sys.params++] = Slvs_MakeParam(7, g, qz);
  sys.entity[sys.entities++] = Slvs_MakeNormal3d(-102, g, 4, 5, 6, 7);

  sys.entity[sys.entities++] = Slvs_MakeWorkplane(-200, g, -101, -102);

  /* Now create a second group. We'll solve group 2, while leaving group 1
     * constant; so the workplane that we've created will be locked down,
     * and the solver can't move it. */
  g = 2;
  /* These points are represented by their coordinates (u v) within the
     * workplane, so they need only two parameters each. */

  sys.entity[sys.entities++] = Slvs_MakePoint2d(-103, g, -200, 1, 1);
  sys.entity[sys.entities++] = Slvs_MakePoint2d(-104, g, -200, 1, 3);
  sys.entity[sys.entities++] = Slvs_MakePoint2d(-105, g, -200, 3, 1);
  sys.entity[sys.entities++] = Slvs_MakeLineSegment(-106, g,
                                                    -200, -103, -104); //y-axis
  sys.entity[sys.entities++] = Slvs_MakeLineSegment(-107, g,
                                                    -200, -103, -105); //x-axis

  Slvs_hParam ph = 11;

  //Input points (nPts) are processed. For each valid point (not NaN), parameters and entities are created and added to the system.

  float *buf_pt_start = p_ptr;
  int p_start = sys.params;
  for (int i = 0; i < nPts; i++)
  {
    if (isnan((float)*p_ptr))
    {
      p_ptr += 3;
      continue;
    }

    sys.param[sys.params++] = Slvs_MakeParam(ph++, g, (float)*p_ptr++);
    sys.param[sys.params++] = Slvs_MakeParam(ph++, g, (float)*p_ptr++);
    sys.entity[sys.entities++] = Slvs_MakePoint2d(i, g, -200, ph - 1, ph - 2);
    p_ptr += 1;
  }

  // Links (nLinks) are processed. Line segments or arcs of a circle are added to the system based on the input.

  for (int i = 0; i < nLinks; i++)
  {

    switch ((int)*l_ptr++)
    {
    case 0:
      sys.entity[sys.entities++] = Slvs_MakeLineSegment((int)*(l_ptr + 2), g,
                                                        -200, (int)*l_ptr, (int)*(l_ptr + 1));
      break;
    case 1:
      sys.entity[sys.entities++] = Slvs_MakeArcOfCircle((int)*(l_ptr + 3), g, -200, -102,
                                                        (int)*(l_ptr + 2), (int)*(l_ptr), (int)*(l_ptr + 1));
      break;
    default:
      break;
    }

    l_ptr += 4;
  }

  //Constraints are added to the system. Coincident points and alternative constraints are created based on the input.

  int c_pre = 1;
  sys.constraint[sys.constraints++] = Slvs_MakeConstraint(
      c_pre++, 2,
      SLVS_C_POINTS_COINCIDENT,
      -200,
      -1,
      -101, geomStartIdx, -1, -1);

  for (int c_id = c_pre; c_id < c_pre + nConst; c_id++, c_ptr += 8)
  {
    if ((int)*c_ptr >= 34)
    {
      sys.constraint[sys.constraints++] = Slvs_MakeConstraint_Alt(
          c_id, g,
          30 + 100000,
          -200,
          *(c_ptr + 1),
          (int)*(c_ptr + 2), (int)*(c_ptr + 3), (int)*c_ptr == 34 ? -106: -107, (int)*(c_ptr + 5), (int)*(c_ptr + 6), (int)*(c_ptr + 7));
    }
    else
    {
      sys.constraint[sys.constraints++] = Slvs_MakeConstraint_Alt(
          c_id, g,
          (int)*c_ptr + 100000,
          -200,
          *(c_ptr + 1),
          (int)*(c_ptr + 2), (int)*(c_ptr + 3), (int)*(c_ptr + 4), (int)*(c_ptr + 5), (int)*(c_ptr + 6), (int)*(c_ptr + 7));
    }
  }

  /* And solve. */
  Slvs_Solve(&sys, g);

  if (sys.result == SLVS_RESULT_OKAY)
  {
    // printf("solved okay\n");

    for (int i = 0; i < nPts; i++)
    {
      if (isnan((float)*buf_pt_start))
      {
        buf_pt_start += 3;
        continue;
      }
      *buf_pt_start++ = (float)sys.param[p_start++].val;
      *buf_pt_start++ = (float)sys.param[p_start++].val;
      buf_pt_start += 1;
    }
  }
  else
  {
    int i;
    printf("solve failed: problematic constraints are:");
    for (i = 0; i < sys.faileds; i++)
    {
      printf(" %d", sys.failed[i]);
    }
    printf("\n");
    if (sys.result == SLVS_RESULT_INCONSISTENT)
    {
      printf("system inconsistent\n");
    }
    else
    {
      printf("system nonconvergent\n");
    }
  }
  sys.params = sys.constraints = sys.entities = 0;
  return 0;
}

int main(int argc, char *argv[])
{
  sys.param = CheckMalloc(500 * sizeof(sys.param[0]));
  sys.entity = CheckMalloc(500 * sizeof(sys.entity[0]));
  sys.constraint = CheckMalloc(500 * sizeof(sys.constraint[0]));

  sys.failed = CheckMalloc(500 * sizeof(sys.failed[0]));
  sys.faileds = 500;

  // printf("hello\n");
  return 0;
}
