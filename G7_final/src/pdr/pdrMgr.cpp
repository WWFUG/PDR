/****************************************************************************
  FileName     [ pdrMgr.cpp ]
  PackageName  [ pdr ]
  Synopsis     [ Define PDR main functions ]
  Author       [ SillyDuck ]
  Copyright    [ Copyright(c) 2016 DVLab, GIEE, NTU, Taiwan ]
 ****************************************************************************/
#define showinfo 0

#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdio>
#include <stdlib.h>
#include <cassert>
#include <climits>
#include <cmath>
#include <unistd.h>
#include <queue>
#include <vector>
#include <algorithm>

#include "v3Msg.h"
#include "v3NtkUtil.h"
#include "v3SvrPDRSat.h"
#include "v3NtkHandler.h" // MODIFICATION FOR SoCV BDD
#include "v3Ntk.h"
#include "PDRDef.h"
#include "reader.h"
#include "pdrMgr.h"

using namespace std;

unsigned Cube::_L = 0;
unsigned Cube::_I = 0;

// Compare class for priority_queue
class TCubeCmp {
 public:
  bool operator() (const TCube lhs, const TCube rhs) const {
    return lhs._frame > rhs._frame;
  }
};

// Main verification starts here
void PDRMgr::verifyProperty(const string& name, const V3NetId& monitor) {
  _ntk = new V3Ntk();
  *_ntk = *(v3Handler.getCurHandler()->getNtk());
  SatProofRes pRes;

  V3SvrPDRSat* satSolver = new V3SvrPDRSat(_ntk, false, false);

  // monitor is a V3NetId,
  // Please figure out how V3 present a circuit
  // by V3Ntk and V3NetId in src/ntk/V3Ntk.h and V3Type.h
  satSolver->setMonitor(monitor);

  pRes.setMaxDepth(100);
  pRes.setSatSolver(satSolver);
  double runTime = 0;
  double ctime = (double)clock() / CLOCKS_PER_SEC;

  PDR(monitor, pRes);

  runTime += (((double)clock() / CLOCKS_PER_SEC) - ctime);
  pRes.reportResult(name);
  

  if(pRes.isFired()){
    unsigned i=0;
    cout << "Counter Example: \n";
    Cube* cur = _head;
    while(cur){
      cout << i << ": " << cur->inputAssign() << endl;
      cur = cur->getNext();
      ++i;
    }
  }
  
  cerr << "runtime: " << runTime << endl;
  delete satSolver; delete _ntk;
  reset();
}

void PDRMgr::reset() {
  _ntk = NULL;
  for(unsigned i=0; i<_F.size(); ++i){
    for(unsigned j=0; j<_F[i].size(); ++j)
      delete _F[i][j];
  }
  //F = NULL;
  _F.clear();
  _head = NULL;
}

void PDRMgr::buildAllNtkVerifyData(const V3NetId& monitor) {
  // Added all circuit constraint to SAT solver here.

  for (uint32_t i = 0; i < _ntk->getLatchSize(); ++i)
     Z->addBoundedVerifyData(_ntk->getLatch(i), 0);
  for (uint32_t i = 0; i < _ntk->getLatchSize(); ++i)
     Z->addBoundedVerifyData(_ntk->getLatch(i), 1);
  Z->addBoundedVerifyData(monitor, 0);
  Z->initValue3Data();
}

bool PDRMgr::PDR(const V3NetId& monitor, SatProofRes& pRes) {
  // assume no inout
  if (_ntk->getInoutSize()) { assert(0); }
  Z = pRes.getSatSolver();
  L = _ntk->getLatchSize();
  I = _ntk->getInputSize();
  Cube::_L = L;
  Cube::_I = I;

  //F = new vector<vector<Cube*>*>();
  _F = vector< vector<Cube*> >();
  Z->setFrame(_F);
  buildAllNtkVerifyData(monitor);
  // this is F_inf
  // which means the cube that will never be reached
  // in any time frame
  // watch out that _frame of cube in this Frame will be INT_MAX
  _F.push_back(vector<Cube*>());

  depth = 0;
  newFrame(); // F[0]

  Z->addInitiateState();
  Z->setMonitor(monitor);

  // PDR Main function
  while (true) {
    // find bad cube, check SAT? (!P & R_k)
    Cube* cube = Z->getBadCube(depth);

    if ( cube ) {
      TCube t(cube, depth);
      // Counter example found
      if (!recursiveBlockCube(t)) {
        pRes.setFired(0);
        return true;
      }
    } else {
      // depth will only be increased here
      depth++;
      newFrame();
      // Fixed point
      if (propagateBlockedCubes(pRes)) {
        return false;
      }
    }
  }
}

bool PDRMgr::recursiveBlockCube(TCube s0){
  if(debug) cout << "Recursive Block Cubes ... "<< endl;
  priority_queue<TCube, vector<TCube>, TCubeCmp> Q;
  Q.push(s0);
  //_cex.clear();

  while (!Q.empty()) {
    TCube s = Q.top();
    Q.pop();
    // reach R0, counter example found
    if (s._frame == 0) {
      //TODO
      if(!_head){ 
        (s._cube)->setNext(NULL);
        _head = s._cube;
      }
      return false;
    }
    // block s
    if(debug) cout << "  Block Cube in frame " << s._frame << endl;
    if (!isBlocked(s)) {
      assert(!(Z->isInitial(s._cube)));
      TCube z = Z->solveRelative(s, 1);
      if (z._frame != -1) {
        // UNSAT, s is blocked

        z = generalize(z);  // UNSAT generalization

        while (z._frame < (int)(depth - 1)) {
          //cout << " z cube in recBlockCube UNSAT " << z._cube  << endl; 
          TCube t = Z->solveRelative(next(z), 0 );
          if (t._frame != -1) { z = t;}
          else break;
        }
        if(debug) cout << "  Add Blocked cube in frame " << z._frame << endl;
        //cout << " z cube in recBlockCube UNSAT " << z._cube  << endl;
        addBlockedCube(z);
        if((s._frame < (int)depth) && (z._frame != INT_MAX)) {
          TCube a = next(s);
          Q.push(a);
          if(debug) cout << "  Add Blocked cube in next frame " << s._frame+1 << endl;
        }
      } else {

        z._frame = s._frame - 1;

        _head = z._cube;
        Q.push(z);
        Q.push(s); // put it in queue again

      }

    }
  }
  return true;
}

bool PDRMgr::isBlocked(TCube s) {

  for (unsigned d = s._frame; d < _F.size(); ++d) {
    for (unsigned i = 0; i < _F[d].size(); ++i) {
      if ( _F[d][i]->subsumes(s._cube)) {
        return true;
      }
    }
  }
  return Z->isBlocked(s);
}

TCube PDRMgr::generalize(TCube s) {
  //UNSAT generalization

  for (unsigned i = 0; i < L; ++i) {
    if (s._cube->_latchValues[i]._dontCare == 1) continue;
    else s._cube->_latchValues[i]._dontCare = 1;

    if (!(Z->isInitial(s._cube))) {
      TCube t = Z->solveRelative( s, 0 );
      if( t._frame==-1 ) s._cube->_latchValues[i]._dontCare = 0;
      else s = t;
    }
    else s._cube->_latchValues[i]._dontCare = 0;
  }
  return s;
}

bool PDRMgr::propagateBlockedCubes(SatProofRes& pRes) {


  //cout << "Propogate cubes "  << endl;
  for (unsigned k = 1; k < depth; ++k) {
    unsigned len =_F[k].size(); // start of removeSubsumed
    for (unsigned i = 0; i < _F[k].size(); ++i) {
      //cout << "Cube in frame " << _F[k][i] << endl;
      TCube s = Z->solveRelative( TCube(_F[k][i], k+1), 2 );
      if (s._frame != -1) { addBlockedCube(s);}
      //else ++i;
    }
    if ( _F[k].size() == 0){
      pRes.setProved(k);
      return true;
    }
  }
  return false;
}

void PDRMgr::newFrame(bool force) {

  if (force || depth >= _F.size() - 1) {
    unsigned n = _F.size();
    _F.push_back(vector<Cube*>());
    _F[n].swap(_F[n-1]);
    Z->newActVar();
    assert(Z->_actVars.size() == _F.size() - 1); // Frame_inf doesn't need ActVar
  }

  assert ( depth <= _F.size()-2 ) ;

}

void PDRMgr::addBlockedCube(TCube s) {
  assert(s._frame != -1);

  if((unsigned)s._frame == _F.size() - 1){
    newFrame(true);
  }
  int l = s._frame;
  int r = _F.size()-1;
  unsigned k = (unsigned)(l > r ? r : l);
  for (unsigned d = 1; d <= k; ++d) {
    for (unsigned i = 0; i < _F[d].size(); ) {
      if ( s._cube->subsumes(_F[d][i]) ){
        delete _F[d][i];
        _F[d][i] = _F[d].back();
        _F[d].pop_back();
      } else {
        i++;
      }
    }
  }
  _F[k].push_back(s._cube);

  Z->blockCubeInSolver(s);
}

TCube PDRMgr::next(const TCube& s){
  return TCube(s._cube, s._frame + 1);
}

void PDRMgr::getInputAssign(V3BitVecX& inV){
  inV.resize(Z->_I);
  for (int i = Z->_I-1; i >= 0; --i) {
    if (Z->existVerifyData(_ntk->getInput(i), 0)) {
      V3BitVecX data = Z->getDataValue(_ntk->getInput(i), 0);
      //cout << data[0] << endl;
      if(data[0]=='1') inV.set1(i);
      else inV.set0(i);
    } else inV.setX(i);
  }
}
