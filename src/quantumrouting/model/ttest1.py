def Solver(H,J):
    from dwave_qbsolv import QBSolv
    h=dict(enumerate(H.flatten(),0))
    j=dict(((i,j),J[i][j]) for i in range(len(J)) for j in range(len(J[0])) if i>j and J[i][j] !=0)
    response = QBSolv().sample_ising(h,j)
    sample=list(response.samples())
    value=list(sample[0].values())
    return value

def test1():
    a = 5
    a = a+1
    return a 
