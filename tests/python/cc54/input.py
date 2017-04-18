#! CCSD dipole with user-specified basis set
import psi4

h2o = psi4.geometry("""
  0 1
  H
  O 1 0.957
  H 2 0.957 1 104.5
""")

psi4.set_options({'freeze_core': 'false'})

psi4.basis_helper("""
# Sadlej-pVTZ
spherical
****
H 0
S 4 1.00
        33.8650140000           0.0060680000
         5.0947880000           0.0453160000
         1.1587860000           0.2028460000
         0.3258400000           0.5037090000
S 1 1.00
         0.1027410000           1.0000000000
S 1 1.00
         0.0324000000           1.0000000000
P 2 1.00
         1.1588000000           0.1884400000
         0.3258000000           0.8824200000
P 2 1.00
         0.1027000000           0.1178000000
         0.0324000000           0.0042000000
****
C 0
S 5 1.00
      5240.6353000000           0.0009370000
       782.2048000000           0.0072280000
       178.3508300000           0.0363440000
        50.8159420000           0.1306000000
        16.8235620000           0.3189310000
S 2 1.00
         6.1757760000           0.4387420000
         2.4180490000           0.2149740000
S 1 1.00
         0.5119000000           1.0000000000
S 1 1.00
         0.1565900000           1.0000000000
S 1 1.00
         0.0479000000           1.0000000000
P 4 1.00
        18.8418000000           0.0138870000
         4.1592400000           0.0862790000
         1.2067100000           0.2887440000
         0.3855400000           0.4994110000
P 1 1.00
         0.1219400000           1.0000000000
P 1 1.00
         0.0385680000           1.0000000000
D 2 1.00
         1.2067000000           0.2628500000
         0.3855000000           0.8043000000
D 2 1.00
         0.1219000000           0.6535000000
         0.0386000000           0.8636000000
****
O 0
S 5 1.00
     10662.2850000000           0.0007990000
      1599.7097000000           0.0061530000
       364.7252600000           0.0311570000
       103.6517900000           0.1155960000
        33.9058050000           0.3015520000
S 2 1.00
        12.2874690000           0.4448700000
         4.7568050000           0.2431720000
S 1 1.00
         1.0042710000           1.0000000000
S 1 1.00
         0.3006860000           1.0000000000
S 1 1.00
         0.0900300000           1.0000000000
P 4 1.00
        34.8564630000           0.0156480000
         7.8431310000           0.0981970000
         2.3062490000           0.3077680000
         0.7231640000           0.4924700000
P 1 1.00
         0.2148820000           1.0000000000
P 1 1.00
         0.0638500000           1.0000000000
D 2 1.00
         2.3062000000           0.2027000000
         0.7232000000           0.5791000000
D 2 1.00
         0.2149000000           0.7854500000
         0.0639000000           0.5338700000
****
""")

ccsd_e, wfn = psi4.property('ccsd',properties=['dipole'],return_wfn=True)
psi4.oeprop(wfn,"DIPOLE", "QUADRUPOLE", title="(OEPROP)CC")
psi4.compare_values(psi4.get_variable("(OEPROP)CC DIPOLE X"), 0.000000000000,6,"CC DIPOLE X")             #TEST
psi4.compare_values(psi4.get_variable("(OEPROP)CC DIPOLE Y"), 0.000000000000,6,"CC DIPOLE Y")             #TEST
psi4.compare_values(psi4.get_variable("(OEPROP)CC DIPOLE Z"),-1.840334899884,6,"CC DIPOLE Z")             #TEST
psi4.compare_values(psi4.get_variable("(OEPROP)CC QUADRUPOLE XX"),-7.864006962064,6,"CC QUADRUPOLE XX")   #TEST
psi4.compare_values(psi4.get_variable("(OEPROP)CC QUADRUPOLE XY"), 0.000000000000,6,"CC QUADRUPOLE XY")   #TEST
psi4.compare_values(psi4.get_variable("(OEPROP)CC QUADRUPOLE XZ"), 0.000000000000,6,"CC QUADRUPOLE XZ")   #TEST
psi4.compare_values(psi4.get_variable("(OEPROP)CC QUADRUPOLE YY"),-4.537386915305,6,"CC QUADRUPOLE YY")   #TEST
psi4.compare_values(psi4.get_variable("(OEPROP)CC QUADRUPOLE YZ"), 0.000000000000,6,"CC QUADRUPOLE YZ")   #TEST
psi4.compare_values(psi4.get_variable("(OEPROP)CC QUADRUPOLE ZZ"),-6.325836255265,6,"CC QUADRUPOLE ZZ")   #TEST
psi4.core.print_variables()
