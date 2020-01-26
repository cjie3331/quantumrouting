
H={0:-1,1:1,2:-1}
J={(0,1):-1,(1,2):-1}

def Solver(H,J):
    from dwave_qbsolv import QBSolv
    response = QBSolv().sample_ising(H,J)
    sample=list(response.samples())
    return sample

def test1():
    a = 5
    a = a+1
    return a 
