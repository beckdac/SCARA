#ifndef __KINEMATICS_H__
#define __KINEMATICS_H__

// arm lengths
// arm from shoulder to elbow
#define L1_MM 100.0
// forearm from elbow to wrist
#define L2_MM 75.0

int kinematicsForward(float *x, float *y, float L1, float L2, float S, float E);
int kinematicsInverse(float x, float y, float L1, float L2, float *S, float *E);
int kinematicsTest(int argc, char *argv[]);

#endif /* __KINEMATICS_H__ */
