def Solver(H,J):
    from dwave_qbsolv import QBSolv
    from dwave.system.samplers import DWaveSampler
    from dwave.system.composites import EmbeddingComposite
    from dwave.cloud import Client
    import itertools
    import random
    sampler = EmbeddingComposite(DWaveSampler(token='DEV-34ebdd33f77f73904ed58bac612191ccdce89841'))
    h=dict(enumerate(H.flatten(),0))
    j=dict(((i,j),J[i][j]) for i in range(len(J)) for j in range(len(J[0])) if i>j and J[i][j] !=0)
    response = QBSolv().sample_ising(h,j,solver=sampler)
    sample=list(response.samples())
    print(sample)
    return sample

def test1():
    a = 5
    a = a+1
    return a 
