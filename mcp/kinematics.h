#ifndef __KINEMATICS_H__
#define __KINEMATICS_H__

int kinematics_forward(float *x, float *y, float L1, float L2, float S, float E);
int kinematics_inverse(float x, float y, float L1, float L2, float *S, float *E);
int kinematics_test(int argc, char *argv[]);

#endif /* __KINEMATICS_H__ */
