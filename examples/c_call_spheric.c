#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cint_bas.h"

int factorial(int n)
{
        int i, fact = 1;
        for (i = 1; i <= n; i++) {
                fact *= i;
        }
        return fact;
}

// normalization factor of function r^n e^{-a r^2}
double gto_norm(int n, double a)
{
        return pow(2, (2*n+3)) * factorial(n+1) * pow((2*a), (n+1.5)) \
                / (factorial(2*n+2) * sqrt(M_PI));
}

int main()
{
        int natm = 2;
        int nbas = 3;
        // ATM_SLOTS = 6; BAS_SLOTS = 8;
        int *atm = malloc(sizeof(int) * natm * ATM_SLOTS);
        int *bas = malloc(sizeof(int) * nbas * BAS_SLOTS);
        double *env = malloc(sizeof(double) * 10000);

        int i, n, off;
        off = PTR_ENV_START;
        for (i = 0; i < natm; i++) {
                atm[CHARGE_OF + ATM_SLOTS * i] = i + 1;
                atm[PTR_COORD + ATM_SLOTS * i] = off;
                env[off + 0] = i;
                env[off + 1] = i;
                env[off + 2] = i;
                off += 3;
        }

        n = 0;

        /* basis #0, slot KAPPA_OF is useless */
        bas[ATOM_OF  + BAS_SLOTS * n]  = 0;
        bas[ANG_OF   + BAS_SLOTS * n]  = 1;
        bas[NPRIM_OF + BAS_SLOTS * n]  = 1;
        bas[NCTR_OF  + BAS_SLOTS * n]  = 1;
        bas[PTR_EXP  + BAS_SLOTS * n]  = off;
        env[off + 0] = 1.;
        bas[PTR_COEFF+ BAS_SLOTS * n] = off + 1;
        env[off + 1] = 1. * gto_norm(bas[ANG_OF+BAS_SLOTS*n], env[bas[PTR_EXP+BAS_SLOTS*n]]);
        off += 2;
        n++;

        /* basis #1, * 2 primitive-GTO -> 2 contracted-GTO */
        bas[ATOM_OF  + BAS_SLOTS * n]  = 0;
        bas[ANG_OF   + BAS_SLOTS * n]  = 2;
        bas[NPRIM_OF + BAS_SLOTS * n]  = 2;
        bas[NCTR_OF  + BAS_SLOTS * n]  = 2;
        bas[PTR_EXP  + BAS_SLOTS * n]  = off;
        env[off + 0] = 3.;
        env[off + 1] = 5.;
        bas[PTR_COEFF+ BAS_SLOTS * n] = off + 2;
        env[off + 2] = 1. * gto_norm(bas[ANG_OF+BAS_SLOTS*n], env[bas[PTR_EXP+BAS_SLOTS*n]])  ;
        env[off + 3] = 2. * gto_norm(bas[ANG_OF+BAS_SLOTS*n], env[bas[PTR_EXP+BAS_SLOTS*n]+1]);
        env[off + 4] = 4. * gto_norm(bas[ANG_OF+BAS_SLOTS*n], env[bas[PTR_EXP+BAS_SLOTS*n]])  ;
        env[off + 5] = 8. * gto_norm(bas[ANG_OF+BAS_SLOTS*n], env[bas[PTR_EXP+BAS_SLOTS*n]+1]);
        off += 6;
        n++;

        /* basis #2 with kappa < 0  => f_5/2, */
        bas[ATOM_OF  + BAS_SLOTS * n]  = 1;
        bas[ANG_OF   + BAS_SLOTS * n]  = 3;
        bas[NPRIM_OF + BAS_SLOTS * n]  = 1;
        bas[NCTR_OF  + BAS_SLOTS * n]  = 1;
        bas[PTR_EXP  + BAS_SLOTS * n]  = off;
        env[off + 0] = 1;
        bas[PTR_COEFF+ BAS_SLOTS * n] = off + 1;
        env[off + 1] = 1 * gto_norm(bas[ANG_OF+BAS_SLOTS*n], env[bas[PTR_EXP+BAS_SLOTS*n]]);
        off += 2;
        n++;

        /*
         * call one-electron spinor integrals
         */
        int j, k, l;
        int di, dj, dk, dl;
        int shls[4];
        double *buf;

        i = 0; shls[0] = i; di = cgtos_spheric(i, bas);
        j = 1; shls[1] = j; dj = cgtos_spheric(j, bas);
        buf = malloc(sizeof(double) * di * dj);
        if (0 != cint1e_nuc_sph(buf, shls, atm, natm, bas, nbas, env)) {
                printf("This gradient integral is not 0.\n");
        }
        free(buf);

        /*
         * call two-electron spinor integrals
         */
        i = 0; shls[0] = i; di = cgtos_spheric(i, bas);
        j = 1; shls[1] = j; dj = cgtos_spheric(j, bas);
        k = 2; shls[2] = k; dk = cgtos_spheric(k, bas);
        l = 2; shls[3] = l; dl = cgtos_spheric(l, bas);
        buf = malloc(sizeof(double) * di * dj * dk * dl);
        if (0 != cint2e_sph(buf, shls, atm, natm, bas, nbas, env)) {
                printf("This gradient integral is not 0.\n");
        }
        free(buf);

        free(atm);
        free(bas);
        free(env);
}
