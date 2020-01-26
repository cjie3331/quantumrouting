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

#ifndef QUANTUM_ROUTING_TABLE_H
#define QUANTUM_ROUTING_TABLE_H
#include <Python.h>
//#include "/home/jessica/.local/lib/python2.7/site-packages/numpy/core/include/numpy/arrayobject.h"
#include "ns3/node.h"
#include "ns3/ipv4-route.h"
#include "ns3/output-stream-wrapper.h"
#include <list>
#include <vector>
using namespace std;
namespace ns3{

typedef struct{
	double* H_core;
	double** J_core;
}coreArray;

typedef struct{
	double* penalty;
	double** interaction;
	int* var_size;
	int varnum;
	int msize;
}effArray;

typedef struct{
	double* H_core;
	double** J_core;
	double* H_prob;
	double** J_prob;
	int qbits;
}dwArray;

typedef struct{
	int y;
	int markflow;
	int pathind;
	int srnodeid;
}bundle;

typedef struct
   {
     Ptr<Node> node;
     Ipv4Address addr;
   }
 SpNodeEntry;
class QuantumRoutingTable : public Object
{
public:

  QuantumRoutingTable ();
  virtual ~QuantumRoutingTable ();

  static TypeId GetTypeId ();
  void AddRoute (Ipv4Address srcAddr, Ipv4Address relayAddr, Ipv4Address dstAddr);
  void AddNode (Ptr<Node> node, Ipv4Address addr);
  Ipv4Address LookupRoute (Ipv4Address srcAddr, Ipv4Address dstAddr,int flowID);
  void UpdateRoute (double txRange);
  void write3pathTable();
  double GetDistance (Ipv4Address srcAddr, Ipv4Address dstAddr);
  void readAdjMatrix(std::string adj_mat_file_name);
  void Print (Ptr<OutputStreamWrapper> stream) const;
  void printAllPaths(int src);
  void printAllPathsUtil(int src, int* visited, int* pcollector);
  void printStat();
  void callPython(dwArray dw_struct);
  dwArray makeDWencoding(effArray input);
  int getvarindex(int i, int* var_size, int varnum);
  bundle checkvalidity(int routeid, double* flowm);
  void writeEdgeTable();
  void getEdgeNum();
  effArray  makeEffArray(double* flowm);
  coreArray makeVarDW(int* var_size, int count, int nqbits);
  void updateRoutingTable();
  void routineCall(double** flowm,int num);
  std::vector<SpNodeEntry> m_nodeTable;
  void callPython2(dwArray dw_struct);
  void closePython();
private:
  typedef struct
    {
      Ipv4Address srcAddr;
      Ipv4Address relayAddr;
      Ipv4Address dstAddr;
    }
  SPtableEntry;
  

  
  double DistFromTable (uint16_t i, uint16_t j);
  
  std::list<SPtableEntry> m_sptable;

  PyObject *pName,*pModule,*pFunc;
  int** m_spNext;
  double*   m_spDist;
  vector<vector<bool> > m_adj;
  int nodenum;
  int**		m_route;
  double    m_txRange;
  int destid;
  int path_index;
  int flag;
  int edgenum;
  int** m_edge;
  int d0;
  int E;
  int epson_mp;
  int epson_fs;
  int bw;
  int t;
  double lambda;
  int** edge_count;
  int flownum;
  int *dwOutput;
  double *flowarray;
  int routinecount;
};

}

#endif // QUANTUM_ROUTING_TABLE_H
