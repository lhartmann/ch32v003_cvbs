#! /usr/bin/env python3

Rv = 220
Rs = 390
R0 = 180

def Q(V,S):
    Rs0 = 1/(1/Rs + 1/R0)
    Rv0 = 1/(1/Rv + 1/R0)
    return V*Rs0/(Rs0+Rv) + S*Rv0/(Rv0+Rs)

print(f'V S Q')
print(f'0 0 {Q(0,0)*3.3}')
print(f'0 1 {Q(0,1)*3.3}')
print(f'1 0 {Q(1,0)*3.3}')
print(f'1 1 {Q(1,1)*3.3}')
