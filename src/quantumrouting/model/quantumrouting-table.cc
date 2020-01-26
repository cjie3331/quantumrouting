/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Arizona
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Author: Junseok Kim <junseok@email.arizona.edu> <engr.arizona.edu/~junseok>
 */
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "quantumrouting-table.h"
#include "ns3/mobility-model.h"
#include "/home/jessica/.local/lib/python2.7/site-packages/numpy/core/include/numpy/arrayobject.h"
#include <Python.h>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <math.h>
using namespace std;

NS_LOG_COMPONENT_DEFINE ("QuantumRoutingTable");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (QuantumRoutingTable);

TypeId
QuantumRoutingTable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::QuantumRoutingTable")
    .SetParent<Object> ()
    .AddConstructor<QuantumRoutingTable> ()
    ;
  return tid;
}

void QuantumRoutingTable::readAdjMatrix (std::string adj_mat_file_name)
{
  ifstream adj_mat_file;
  adj_mat_file.open (adj_mat_file_name.c_str (), ios::in);
  if (adj_mat_file.fail ())
    {
      NS_FATAL_ERROR ("File " << adj_mat_file_name.c_str () << " not found");
    }
  vector<vector<bool> >  array;
  int i = 0;
  int n_nodes = 0;

  while (!adj_mat_file.eof ())
    {
      string line;
      getline (adj_mat_file, line);
      if (line == "")
        {
          NS_LOG_WARN ("WARNING: Ignoring blank row in the array: " << i);
          break;
        }

      istringstream iss (line);
      bool element;
      vector<bool> row;
      int j = 0;

      while (iss >> element)
        {
          row.push_back (element);
          j++;
        }

      if (i == 0)
        {
          n_nodes = j;
        }

      if (j != n_nodes )
        {
          NS_LOG_ERROR ("ERROR: Number of elements in line " << i << ": " << j << " not equal to number of elements in line 0: " << n_nodes);
          NS_FATAL_ERROR ("ERROR: The number of rows is not equal to the number of columns! in the adjacency matrix");
        }
      else
        {
          array.push_back (row);
        }
      i++;
    }

  if (i != n_nodes)
    {
      NS_LOG_ERROR ("There are " << i << " rows and " << n_nodes << " columns.");
      NS_FATAL_ERROR ("ERROR: The number of rows is not equal to the number of columns! in the adjacency matrix");
    }

  adj_mat_file.close ();
  m_adj=array;

}

void QuantumRoutingTable::getEdgeNum()
{
	edgenum=0;
	int** edgecount=new int*[nodenum];
	for(int i=0;i<nodenum;i++)
	{
		edgecount[i]=new int[nodenum];
		for(int j=0;j<nodenum;j++)
		{
			edgecount[i][j]=0;
		}
	}
	for(int i=0;i<nodenum;i++)
	{
	    for(int j=(i+1);j<nodenum;j++)
	    {
	    	if(m_adj[i][j]==1){
	    		edgenum++;
	    		edgecount[i][j]=edgenum;
	    		edgecount[j][i]=edgenum;
	    	}
	    }
	}
	edge_count=edgecount;
}
QuantumRoutingTable::QuantumRoutingTable ()
{
  std::cout<<"constructor called"<<endl;
 // m_spNext = 0;
  m_spDist = 0;
  m_txRange = 1000000000000000000;
  d0=87.7058;
  E=50;
  epson_mp=0.0013;
  bw=5;
  t=10;
  lambda=2;
  epson_fs=10;
  destid=1;
  routinecount=0;
  std::string adj_mat_file_name ("src/quantumrouting/examples/6adjm.txt");
  readAdjMatrix(adj_mat_file_name);
  Py_Initialize();
  PyRun_SimpleString("import sys\n");
  PyRun_SimpleString("sys.path.append('src/quantumrouting/model')");
  PyRun_SimpleString("import ttest1;");
  pName = PyString_FromString("ttest1");
  pModule = PyImport_Import(pName);
  pFunc = PyObject_GetAttrString(pModule,(char*)"Solver");

}
QuantumRoutingTable::~QuantumRoutingTable ()
{}

void QuantumRoutingTable::routineCall(double** flowm, int num)
{
       std::cout<<"routineCall reached"<<endl;
       double* flowy=flowm[num];
	   effArray input=makeEffArray(flowy);
	   dwArray output=makeDWencoding(input);
	   std::cout<<"callPython before"<<endl;
	   for(int i=0;i<nodenum;i++)
		   {
			   std::cout<<*(flowm+i)<<" ";
		   }
		   std::cout<<endl;
	   callPython2(output);
	   std::cout<<"callPython after"<<endl;
	   updateRoutingTable();
}
effArray QuantumRoutingTable::makeEffArray(double* flowm)
{
	flowarray=flowm;
	flownum=0;
	for(int i=0;i<nodenum;i++)
	{
		if(flowm[i]!=0){flownum++;}
	}

	int varnum=flownum+edgenum;
	int* var_size=new int[varnum];
	int msize=0;
	for(int i=1;i<=flownum;i++){var_size[i-1]=3;msize=msize+3;}
	for(int i=(flownum+1);i<=varnum;i++){var_size[i-1]=bw;msize=msize+bw;}

	double* penalty=new double[msize];
	double** interaction= new double*[msize];

	for(int i=0;i<msize;i++)
	{
		penalty[i]=0;
		interaction[i]=new double[msize];
		for(int j=0;j<msize;j++)
		{
			interaction[i][j]=0;
		}
	}

	int edgecount;
	int pathnum=nodenum*3;
	for(int i=2;i<=nodenum;i++)
	{
		for(int j=1;j<=nodenum;j++)
		{
			int virtualedgenum=(i-1)*nodenum+j-1;
			if(m_edge[virtualedgenum][0]!=0)
			{
				edgecount=edge_count[i-1][j-1];
                for(int l=1;l<=bw;l++)
                {
                	int m=flownum*3+(edgecount-1)*bw+l;
                	penalty[m-1]=(1-2*bw)*lambda;

                	for(int l1=(l+1);l1<=bw;l1++)
                	{
                		int m1=flownum*3+(edgecount-1)*bw+l1;
                		interaction[m-1][m1-1]=2*lambda;
                	}
                }


				for(int k=1;k<=pathnum;k++)
				{
					int temp=m_edge[virtualedgenum][k-1];
					if(temp==0){break;}
					else{
					bundle bstruct=checkvalidity(temp,flowm);
					if(bstruct.y!=0)
					{
						int m=(bstruct.markflow-1)*3+bstruct.pathind;
						double flowrate=flowm[bstruct.srnodeid-1];
						double dist=DistFromTable(i-1,j-1);
						double val=0;
						if (dist<d0){val=E+epson_fs*dist*dist;}
						else{val=E+epson_mp*dist*dist*dist*dist;}
						penalty[m-1]=penalty[m-1]+t*flowrate*val+lambda*flowrate*flowrate-2*bw*flowrate*lambda;
						for(int l=1;l<=bw;l++){
							int n=flownum*3+(edgecount-1)*bw+l;
							//penalty[n-1]=(1-2*bw)*lambda;
							interaction[m-1][n-1]=interaction[m-1][n-1]+2*flowrate*lambda;
						}

						for(int q=(k+1);q<=pathnum;q++){
							int temp2=m_edge[virtualedgenum][q-1];
							if(temp2==0){break;}
							else{
								bundle bstruct2=checkvalidity(temp2,flowm);
								if(bstruct2.y!=0){
									int n=(bstruct2.markflow-1)*3+bstruct2.pathind;
									double flowrate2=flowm[bstruct2.srnodeid-1];
									interaction[m-1][n-1]=interaction[m-1][n-1]+2*flowrate*flowrate2*lambda;
								}
							}
						}
					}
					}
				}
			}
		}
	}

	effArray eff_struct;
	eff_struct.penalty=penalty;
	eff_struct.interaction=interaction;
	eff_struct.var_size=var_size;
	eff_struct.varnum=varnum;
	eff_struct.msize=msize;
    return eff_struct;
}

bundle QuantumRoutingTable::checkvalidity(int routeid,double* flowm)
{
	bundle bstruct;
	int k=floor((routeid-1)/3)+2;//k is the corresponding node no. starting with 1;
	int pathid=routeid%3;
	if(pathid==0){pathid=3;}
	int y=0;
	int nodepos=0;
	int flownum=0;
	int nodeid=k;
	for(int i=0;i<nodenum;i++)
	{
		if(flowm[i]!=0){
			flownum++;
			if(i==(k-1)){nodepos=flownum;y=1;}
		}
	}
	bstruct.markflow=nodepos;
	bstruct.y=y;
	bstruct.pathind=pathid;
	bstruct.srnodeid=nodeid;
	return bstruct;
}
void QuantumRoutingTable::printStat()
{
	std::cout<<"printer called"<<endl;
	for(int i=0;i<(nodenum-1)*3;i++)
	{
		for(int j=0;j<nodenum;j++)
		{
			std::cout<<m_route[i][j]<<" ";
		}
		std::cout<<endl;
	}

	for(int i=0;i<nodenum;i++)
	{
		for(int j=0;j<nodenum;j++)
		{
			std::cout<<edge_count[i][j]<<" ";
		}
		std::cout<<endl;
	}


}
void 
QuantumRoutingTable::AddRoute (Ipv4Address srcAddr, Ipv4Address relayAddr, Ipv4Address dstAddr)
{
  NS_LOG_FUNCTION (srcAddr << relayAddr << dstAddr);
  SPtableEntry se;
  se.srcAddr = srcAddr;
  se.relayAddr = relayAddr;
  se.dstAddr = dstAddr;
  
  m_sptable.push_back (se);
}
void QuantumRoutingTable::writeEdgeTable()
{
	int pathnum=3*(nodenum-1);
	int rownum=nodenum*nodenum;
	int** edgem=new int*[rownum];
	for(int i=0;i<rownum;i++)
	{
		edgem[i]=new int[pathnum];
		for(int j=0;j<pathnum;j++)
		{
			edgem[i][j]=0;
		}
	}
	int k;
	int p;
	for(int i=1;i<=pathnum;i++)
	{
		for(int j=1;j<nodenum;j++)
		{
			k=m_route[i-1][j-1];
			p=m_route[i-1][j];
			if(p==0){break;}
			else{
				for(int h=1;h<=pathnum;h++)
				{
					int index1=(k-1)*nodenum+(p-1);
					int index2=h-1;
					if(edgem[index1][index2]==0)
					{
						edgem[index1][index2]=i;
						break;
					}
				}
			}
		}
	}

	m_edge=edgem;
}
void
QuantumRoutingTable::AddNode (Ptr<Node> node, Ipv4Address addr)
{
  NS_LOG_FUNCTION ("node: " << node << " addr: " << addr);
  SpNodeEntry sn;
  sn.node = node;
  sn.addr = addr;
  m_nodeTable.push_back (sn);
}

Ipv4Address
QuantumRoutingTable::LookupRoute (Ipv4Address srcAddr, Ipv4Address dstAddr, int flowID)
{
    std::cout<<"look up called and flowID is:"<<flowID<<endl;
	uint16_t i = 0, j = 0;
    Ipv4Address relay;
    int nodeid=flowID;
    std::vector<SpNodeEntry>::iterator it = m_nodeTable.begin ();
    
    for (; it != m_nodeTable.end ();++it)
      {
        if (it->addr == srcAddr)
          {
            break;
          }
        i++;
      }
    it = m_nodeTable.begin ();
    for (; it != m_nodeTable.end ();++it)
      {
        if (it->addr == dstAddr)
          {
            break;
          }
        j++;
      }
    std::cout<<"src ID is :"<<i<<" and "<<"dest ID is"<<j<<endl;
    uint16_t k;
    /*do
      {
        k = j;
        j = m_spNext [i * m_nodeTable.size() + j];
        NS_LOG_INFO ("@@ " << i << " " << j << " " << k);
      }
    while (i != j);
    */

    k=m_spNext[nodeid][i*nodenum+j];
    
    /*for(int p=0;p<nodenum;p++)
    {
    	for(int q=0;q<nodenum;q++)
    	{
    		std::cout<<m_spNext[nodeid][p*nodenum+q]<<" ";
    	}
    	std::cout<<endl;
    }*/

    if (DistFromTable (i, k) > m_txRange)
      {
        NS_LOG_DEBUG ("No Path Exists!");
        std::cout<<"distance is larger than preset range"<<endl;
        return srcAddr;
      }
    std::cout<<"next hop node is :"<<k<<endl;
    SpNodeEntry se = m_nodeTable.at (k);
    return se.addr;
}

dwArray QuantumRoutingTable::makeDWencoding(effArray input)
{
	double* penalty=input.penalty;
	double** interaction=input.interaction;
	int* var_size=input.var_size;
	int varnum=input.varnum;
	int msize=input.msize;
	int nqbits=msize-varnum;
	coreArray core_struct;
	dwArray dw_struct;
	double** J_core=new double*[nqbits];
	double** J_prob=new double*[nqbits];
	double* H_core=new double[nqbits];
	double* H_prob=new double[nqbits];

	for(int i=0;i<nqbits;i++)
	{
		H_core[i]=0;
		H_prob[i]=0;
		J_core[i]=new double[nqbits];
		J_prob[i]=new double[nqbits];
		for(int j=0;j<nqbits;j++)
		{
			J_core[i][j]=0;
			J_prob[i][j]=0;
		}
	}

	for(int i=1;i<=varnum;i++)
	{
		core_struct=makeVarDW(var_size,i,nqbits);
		for(int j=0;j<nqbits;j++)
		{
			H_core[j]=H_core[j]+core_struct.H_core[j];
			for(int k=0;k<nqbits;k++)
			{
				J_core[j][k]=J_core[j][k]+core_struct.J_core[j][k];
			}
		}
	}

	int startnum;
	int varstartnum;
	int startnum1;
	int varstartnum1;
	int startnum2;
	int varstartnum2;

	for(int i=1;i<=msize;i++)
	{
		double* h=new double[nqbits];
		for(int i=0;i<nqbits;i++){h[i]=0;}
		if(penalty[i-1]!=0)
		{
			int varindex=getvarindex(i,var_size,varnum);
			if(varindex==1)
			{
				startnum=0;
				varstartnum=0;
			}else{
				startnum=0;
				varstartnum=0;
				for(int j=1;j<=(varindex-1);j++){
					startnum=startnum+var_size[j-1]-1;
					varstartnum=varstartnum+var_size[j-1];
				}
			}

			if((i-varstartnum)==1){h[startnum]=1;}
			else if ((i-varstartnum)==var_size[varindex-1])
			{
				int m=var_size[varindex-1];
				h[startnum+m-2]=-1;
			}else{
				int k=i-varstartnum;
				h[startnum+k-1]=1;
				h[startnum+k-2]=-1;
			}
		}

		for(int p=0;p<nqbits;p++)
		{
			h[p]=h[p]*0.5*penalty[i-1];
			H_prob[p]=H_prob[p]+h[p];
		}
	}

	for(int i=1;i<=(msize-1);i++)
	{
		for(int j=i;j<=msize;j++)
		{
			if(interaction[i-1][j-1]!=0)
			{
				double* H1=new double[nqbits];
				double** J1=new double*[nqbits];
				for(int k=0;k<nqbits;k++)
				{
					H1[k]=0;
					J1[k]=new double[nqbits];
					for(int q=0;q<nqbits;q++)
					{
						J1[k][q]=0;
					}
				}

				int varindex1=getvarindex(i,var_size,varnum);
				int varindex2=getvarindex(j,var_size,varnum);

				if(varindex1==1)
				{
					startnum1=0;
					varstartnum1=0;
				}else{
					startnum1=0;
					varstartnum1=0;
					for(int k=1;k<=(varindex1-1);k++)
					{
						startnum1=startnum1+var_size[k-1]-1;
						varstartnum1=varstartnum1+var_size[k-1];
					}
				}

				if(varindex2==1)
				{
					startnum2=0;
					varstartnum2=0;
				}else{
					startnum2=0;
					varstartnum2=0;
					for(int k=1;k<=(varindex2-1);k++)
					{
						startnum2=startnum2+var_size[k-1]-1;
						varstartnum2=varstartnum2+var_size[k-1];
					}
				}

				if((i-varstartnum1)==1){
					if((j-varstartnum2)==1){
						J1[startnum1][startnum2]=1;
						H1[startnum1]=1;
						H1[startnum2]=1;
					}else if((j-varstartnum2)==var_size[varindex2-1]){
						int m=var_size[varindex2-1];
						J1[startnum1][startnum2+m-2]=-1;
						H1[startnum1]=1;
						H1[startnum2+m-2]=-1;
					}else{
						int k2=j-varstartnum2;
						J1[startnum1][startnum2+k2-1]=1;
						J1[startnum1][startnum2+k2-2]=-1;
						H1[startnum2+k2-1]=1;
						H1[startnum2+k2-2]=-1;
					}
				}else if((i-varstartnum1)==var_size[varindex1-1]){
					int m1=var_size[varindex1-1];
					if((j-varstartnum2)==1){
						J1[startnum1+m1-2][startnum2]=-1;
						H1[startnum1+m1-2]=-1;
						H1[startnum2]=1;
					}else if((j-varstartnum2)==var_size[varindex2-1]){
						int m2=var_size[varindex2-1];
						J1[startnum1+m1-2][startnum2+m2-2]=1;
						H1[startnum1+m1-2]=-1;
						H1[startnum1+m2-2]=-1;
					}else{
						int k2=j-varstartnum2;
						J1[startnum1+m1-2][startnum2+k2-1]=-1;
						J1[startnum1+m1-2][startnum2+k2-2]=1;
						H1[startnum2+k2-1]=1;
						H1[startnum2+k2-2]=-1;
					}
				}else{
					int k1=i-varstartnum1;
					if((j-varstartnum2)==1){
						J1[startnum1+k1-1][startnum2]=1;
						J1[startnum1+k1-2][startnum2]=-1;
						H1[startnum1+k1-1]=1;
						H1[startnum1+k1-2]=-1;
					}else if((j-varstartnum2)==var_size[varindex2-1]){
						int m2=var_size[varindex2-1];
						J1[startnum1+k1-1][startnum2+m2-2]=-1;
						J1[startnum1+k1-2][startnum2+m2-2]=1;
						H1[startnum1+k1-1]=1;
						H1[startnum1+k1-2]=-1;
					}else{
						int k2=j-varstartnum2;
						J1[startnum1+k1-1][startnum2+k2-1]=1;
						J1[startnum1+k1-1][startnum2+k2-2]=-1;
						J1[startnum1+k1-2][startnum2+k2-1]=-1;
						J1[startnum1+k1-2][startnum2+k2-2]=1;
					}
				}

				for(int p=0;p<nqbits;p++)
				{
					H1[p]=0.25*H1[p]*interaction[i-1][j-1];
					H_prob[p]=H_prob[p]+H1[p];
					for(int q=0;q<nqbits;q++)
					{
						J1[p][q]=0.25*J1[p][q]*interaction[i-1][j-1];
						J_prob[p][q]=J_prob[p][q]+J1[p][q];
					}
				}

			}

		}
	}

	dw_struct.H_core=H_core;
	dw_struct.H_prob=H_prob;
	dw_struct.J_core=J_core;
	dw_struct.J_prob=J_prob;
	dw_struct.qbits=nqbits;
	return dw_struct;
}

void QuantumRoutingTable::updateRoutingTable()
{
	int sourceid[nodenum];
	int pathid[nodenum];
     for(int k=0;k<nodenum;k++)
	{
	   for(int i=0;i<nodenum;i++)
	 {
		for(int j=0;j<nodenum;j++)
		{
			m_spNext[k][i*nodenum+j]=0;
		}
	 }
	}


	for(int i=0;i<nodenum;i++)
	{
		sourceid[i]=0;pathid[i]=0;
	}

	for(int i=0;i<nodenum;i++)
	{
		if(*(flowarray+i)!=0)
		{
			for(int j=0;j<nodenum;j++)
			{
				if(sourceid[j]==0)
				{
					sourceid[j]=i;
					break;
				}
			}

		}

	}



	for(int i=0;i<flownum;i++)
	{
		int m=i*2;
		int n=i*2+1;
		int nodeid = sourceid[i];
		std::cout<<*(dwOutput+m)<<*(dwOutput+n)<<endl;
		if(*(dwOutput+m)==-1 && *(dwOutput+n)==-1)
		{

			pathid[nodeid]=3;
		}
		else if(*(dwOutput+m)==-1 && *(dwOutput+n)==1)
		{
			pathid[nodeid]=2;
		}
		else{
			pathid[nodeid]=1;
		}
	}
	int i=0;
	while(sourceid[i]!=0)
	{
		int nodeid=sourceid[i];
		std::cout<<"update-routing-table node-id is:"<<nodeid<<endl;
		int path_id=pathid[nodeid];
		std::cout<<"path id is :"<<path_id<<endl;
		int routentry[nodenum];
        for(int j=0;j<nodenum;j++)
        {
        	routentry[j]=m_route[(nodeid-1)*3+path_id-1][j];
        	std::cout<<routentry[j]<<" ";
        }
        std::cout<<"above is the route entry"<<endl;
        for(int j=0;j<nodenum;j++)
        {
        	if(routentry[j]!=0 && routentry[j]!=1)
        	{
        		int tempsrc=routentry[j];
        		m_spNext[nodeid][nodenum*(tempsrc-1)]=routentry[j+1]-1;
        	}
        }

        for(int j=(nodenum-1);j>0;j--)
        {
        	if(routentry[j]!=0)
        	{
        		int tempsrc=routentry[j];
        		m_spNext[nodeid][nodenum*(tempsrc-1)+nodeid]=routentry[j-1]-1;
        	}
        }

        for(int k=0;k<nodenum;k++)
        {
        		std::cout<<m_spNext[nodeid][nodeid*nodenum+k]<<" ";
        }
        std::cout<<endl;

		i++;
	}

}
void QuantumRoutingTable::callPython2(dwArray dw_struct)
{
	    PyObject *h,*j;
		PyObject  *pArgs,*pValue;

		//Py_Initialize();
		if(PyArray_API == NULL)
		{
			import_array();
		}

		//pModule = PyImport_Import(pName);
	//	pDict = PyModule_GetDict(pModule);
		//Py_DECREF(pName);

		int qsize=dw_struct.qbits;
		double H[qsize];
		double J[qsize][qsize];
		double lambda=10;
		for(int i=0;i<qsize;i++)
		{
			H[i]=lambda*dw_struct.H_core[i]+dw_struct.H_prob[i];
			for(int j=0;j<qsize;j++)
			{
				J[i][j]=lambda*dw_struct.J_core[i][j]+dw_struct.J_prob[i][j];
			}
		}

	    //float H[3]={-1,1,-1};
	    //float J[3][3]={{0,-1,0},{0,0,-1},{0,0,0}};
	    npy_intp m=qsize;
	    npy_intp dims[2];
	    //int m=3;
	    //int dims[2];
	    dims[0]=qsize;
	    dims[1]=qsize;
	    h=PyArray_SimpleNewFromData(1,&m,NPY_DOUBLE,(void *)H);
	    j=PyArray_SimpleNewFromData(2,dims,NPY_DOUBLE,(void *)J);
		if(pModule != NULL){
		//	pFunc = PyObject_GetAttrString(pModule,(char*)"Solver");
		//	pFunc = PyDict_GetItemString(pDict,"test1");
			if(pFunc && PyCallable_Check(pFunc)){
				pArgs = PyTuple_New(2);
				PyTuple_SetItem(pArgs,0,h);
				PyTuple_SetItem(pArgs,1,j);
				pValue=PyObject_CallObject(pFunc,pArgs);
			//	Py_DECREF(pArgs);
				if(pValue!=NULL){
					//std::cout<<pValue;
					int *p=new int[qsize];
					for(int i=0;i<qsize;i++)
					{
						PyObject *value = PyList_GetItem(pValue,i);
						p[i]=PyFloat_AsDouble(value);
						std::cout<<p[i]<<" ";
					}
					std::cout<<endl;
					dwOutput=p;
					printf("there is result\n");
					Py_DECREF(pValue);
				}
				else{
					Py_DECREF(pFunc);
					Py_DECREF(pModule);
					PyErr_Print();
					fprintf(stderr,"Call failed\n");
				}

			}else{
				if(PyErr_Occurred())
					{
					printf("module not called\n");
					PyErr_Print();
					}
				if(pFunc==NULL)
				{
					printf("pFunc is empty return\n");
				}
				printf("pModule is NULL");
			}

		//	Py_XDECREF(pFunc);
		//	Py_DECREF(pModule);
		}
		else{
			PyErr_Print();
			std::cout<<"Failed to load"<<endl;
		}

	//	Py_Finalize();
	}

void QuantumRoutingTable::closePython()
{
	Py_XDECREF(pFunc);
	Py_DECREF(pName);
	Py_DECREF(pModule);
	Py_Finalize();
}
void QuantumRoutingTable::callPython(dwArray dw_struct)
{
	PyObject *pModule,*pFunc,*h,*j;
	PyObject  *pArgs,*pValue;

	//Py_Initialize();
	if(PyArray_API == NULL)
	{
		import_array();
	}

	pModule = PyImport_Import(pName);
//	pDict = PyModule_GetDict(pModule);
	Py_DECREF(pName);

	int qsize=dw_struct.qbits;
	double H[qsize];
	double J[qsize][qsize];
	double lambda=10;
	for(int i=0;i<qsize;i++)
	{
		H[i]=lambda*dw_struct.H_core[i]+dw_struct.H_prob[i];
		for(int j=0;j<qsize;j++)
		{
			J[i][j]=lambda*dw_struct.J_core[i][j]+dw_struct.J_prob[i][j];
		}
	}

    //float H[3]={-1,1,-1};
    //float J[3][3]={{0,-1,0},{0,0,-1},{0,0,0}};
    npy_intp m=qsize;
    npy_intp dims[2];
    //int m=3;
    //int dims[2];
    dims[0]=qsize;
    dims[1]=qsize;
    h=PyArray_SimpleNewFromData(1,&m,NPY_DOUBLE,(void *)H);
    j=PyArray_SimpleNewFromData(2,dims,NPY_DOUBLE,(void *)J);
	if(pModule != NULL){
		pFunc = PyObject_GetAttrString(pModule,(char*)"Solver");
	//	pFunc = PyDict_GetItemString(pDict,"test1");
		if(pFunc && PyCallable_Check(pFunc)){
			pArgs = PyTuple_New(2);
			PyTuple_SetItem(pArgs,0,h);
			PyTuple_SetItem(pArgs,1,j);
			pValue=PyObject_CallObject(pFunc,pArgs);
		//	Py_DECREF(pArgs);
			if(pValue!=NULL){
				//std::cout<<pValue;
				int *p=new int[qsize];
				for(int i=0;i<qsize;i++)
				{
					PyObject *value = PyList_GetItem(pValue,i);
					p[i]=PyFloat_AsDouble(value);
					std::cout<<p[i]<<" ";
				}
				std::cout<<endl;
				dwOutput=p;
				printf("there is result\n");
				Py_DECREF(pValue);
			}
			else{
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				fprintf(stderr,"Call failed\n");
			}

		}else{
			if(PyErr_Occurred())
				{
				printf("module not called\n");
				PyErr_Print();
				}
			if(pFunc==NULL)
			{
				printf("pFunc is empty return\n");
			}
			printf("pModule is NULL");
		}

	Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else{
		PyErr_Print();
		std::cout<<"Failed to load"<<endl;
	}

	Py_Finalize();
}

int QuantumRoutingTable::getvarindex(int i,int* var_size, int varnum)
{
	int temp=i;
	int varindex=0;
	for(int j=1;j<=varnum;j++)
	{
		temp=temp-var_size[j-1];
		if(temp<=0){
			varindex=j;
			break;
		}
	}
	return varindex;
}

coreArray QuantumRoutingTable::makeVarDW(int* var_size,int count,int nqbits)
{
	coreArray core_struct;
	double* H= new double[nqbits];
	double** J= new double*[nqbits];
	for(int i=0;i<nqbits;i++)
	{
		H[i]=0;
		J[i]=new double[nqbits];
		for(int j=0;j<nqbits;j++)
		{
			J[i][j]=0;
		}
	}

	int varsize=var_size[count-1];
	int start_num;

	if(count==1)
	{
		start_num=1;
	}else{
		int temp=0;
		for(int j=1;j<=(count-1);j++){
			temp=temp+var_size[j-1]-1;
		}
		start_num=temp+1;
	}

	if(varsize>2)
	{
		for(int i=1;i<=(varsize-2);i++)
		{
			J[start_num+i-2][start_num+i-1]=-1;
		}
		H[start_num-1]=1;
		H[start_num+varsize-3]=-1;
	}

	if(varsize==2){
		H[start_num-1]=0;
	}
	core_struct.H_core=H;
	core_struct.J_core=J;
	return core_struct;
}



void QuantumRoutingTable::write3pathTable()
{
	    int** route_m= new int*[(nodenum-1)*3];
		int pathnum=(nodenum-1)*3;
		for(int i=0;i<pathnum;i++)
		{
			route_m[i]=new int[nodenum];
			for(int j=0;j<nodenum;j++)
			{
					route_m[i][j]=0;
			}
		}
		m_route=route_m;
	for(int i=2;i<=nodenum;i++)
	{
		printAllPaths(i);
	}
}

void QuantumRoutingTable::printAllPaths(int src)
{
	int* visited= new int[nodenum];
	int* pcollector= new int[nodenum];

	for(int i=0;i<nodenum;i++)
	{
		visited[i]=0;
		pcollector[i]=0;

	}

	path_index=1;
	flag=0;
	printAllPathsUtil(src,visited,pcollector);
}

void QuantumRoutingTable::printAllPathsUtil(int src, int* visited, int* pcollector)
{
	visited[src-1]=1;
	pcollector[path_index-1]=src;
	path_index++;

	if(src==destid)
	{
		for(int i=0;i<(nodenum-1)*3;i++)
		{
					if(m_route[i][0]==0)
					{
						for(int j=0;j<nodenum;j++)
						{
							m_route[i][j]=pcollector[j];
						}
						break;
					}
		}

	}else{
		for(int i=1;i<=nodenum;i++)
		{
			if(m_adj[src-1][i-1]==1 && visited[i-1]==0)
			{
				printAllPathsUtil(i,visited,pcollector);
				if (flag==1) {break;}
			}
		}
	}

	int count=0;
	for(int i=0;i<(nodenum-1)*3;i++)
	{
		if(m_route[i][0]==0)
		{
				break;
		}
		count++;
	}

	int k=count%3;
    if (k==0){flag=1;}
    path_index=path_index-1;
    visited[src-1]=0;
}

// Find shortest paths for all pairs using Floyd-Warshall algorithm 
void 
QuantumRoutingTable::UpdateRoute (double txRange)
{
  NS_LOG_FUNCTION ("");
  nodenum=m_nodeTable.size();
  m_spNext=new int*[nodenum];
  for(int i=0;i<nodenum;i++)
  {
	  m_spNext[i]=new int[nodenum*nodenum];

	  for(int j=0;j<nodenum*nodenum;j++)
	  {
		  m_spNext[i][j]=0;
	  }
  }

  /*m_txRange = txRange;
  uint16_t n = m_nodeTable.size(); // number of nodes
  uint16_t i, j, k; //loop counters
  double distance;

  //initialize data structures
  if (m_spNext != 0)
    {
      delete m_spNext;
    }
  if (m_spDist != 0)
    {
      delete m_spDist;
    }

  double* dist = new double [n * n];
  uint16_t* pred = new uint16_t [n * n];

  //algorithm initialization
  for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
        {
          if (i == j)
            {
              dist [i * n + j] = 0;
            }
          else
            {
              distance = DistFromTable (i, j);
              if (distance > 0 && distance <= txRange)
                {
	                //dist [i * n + j] = distance; // shortest distance
                  dist [i * n + j] = 1; // shortest hop
                }
              else
                {
	                dist [i * n + j] = HUGE_VAL;
                }
              pred [i * n + j] = i;
            }
        }
    }
    
  // Main loop of the algorithm
  for (k = 0; k < n; k++)
    {
      for (i = 0; i < n; i++)
        {
          for (j = 0; j < n; j++)
            {
              distance = std::min (dist[i * n + j], dist[i * n + k] + dist[k * n + j]);
              if (distance != dist[i * n + j])
                {
                  dist[i * n + j] = distance;
                  pred[i * n + j] = pred[k * n + j];
                }
            }
        }
    }
    
  m_spNext = pred; // predicate matrix, useful in reconstructing shortest routes
  m_spDist = dist;
  
  string str;
  for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
      {
        str.append (boost::lexical_cast<string>( pred[i * n + j] ));
        str.append (" ");
      }
      NS_LOG_INFO (str);
      str.erase (str.begin(), str.end());
    }
    */
}

// Get direct-distance between two nodes
double
QuantumRoutingTable::GetDistance (Ipv4Address srcAddr, Ipv4Address dstAddr)
{
    uint16_t i = 0, j = 0;
    uint16_t n = m_nodeTable.size(); // number of nodes
    Ipv4Address relay;
    std::vector<SpNodeEntry>::iterator it = m_nodeTable.begin ();
    
    for (; it != m_nodeTable.end ();++it)
      {
        if (it->addr == srcAddr)
          {
            break;
          }
        i++;
      }
    it = m_nodeTable.begin ();
    for (; it != m_nodeTable.end ();++it)
      {
        if (it->addr == dstAddr)
          {
            break;
          }
        j++;
      }
    
    return m_spDist [i * n + j];
}

double 
QuantumRoutingTable::DistFromTable (uint16_t i, uint16_t j)
{
  Vector pos1, pos2;
  SpNodeEntry se;
  double dist;
  
  se = m_nodeTable.at (i);
  pos1 = (se.node)->GetObject<MobilityModel> ()->GetPosition ();
  se = m_nodeTable.at (j);
  pos2 = (se.node)->GetObject<MobilityModel> ()->GetPosition ();
  
  dist = pow (pos1.x - pos2.x, 2.0) + pow (pos1.y - pos2.y, 2.0) + pow (pos1.z - pos2.z, 2.0);
  return sqrt (dist);
}

void
QuantumRoutingTable::Print (Ptr<OutputStreamWrapper> stream) const
{
  NS_LOG_FUNCTION ("");
  // not implemented
}

} // namespace ns3
