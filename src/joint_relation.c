/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Project9 contributors.
 * Extracted from the measured Project9 author-baseline extension.
 * See CORE_PROVENANCE.json and THIRD_PARTY_NOTICES.md.
 * This public subset exposes local relation support only, not the complete
 * SAT-integrated solver, explanations, search or experimental workflow.
 */
#include "joint_relation.h"
#include <assert.h>
#include <string.h>
static const int PopOps[5]={2,4,6,8,14};
static unsigned RowRanges[17][17][3],RowCostMasks[129];static int RowRangeN;
static void P9ClosedSupport(int N,unsigned truth,unsigned ops,unsigned *z,unsigned *o,unsigned *d,unsigned *support,unsigned *plain,int budget,int charged0,int charged1){
 if(RowRangeN!=N){
  memset(RowRanges,0,sizeof(RowRanges));memset(RowCostMasks,0,sizeof(RowCostMasks));RowRangeN=N;
  for(int cap=0;cap<=128;cap++)for(int a=1;a<=N/2;a++)if(2*a*(N-a)<=cap)RowCostMasks[cap]|=1u<<a;
  for(int L=0;L<=N;L++)for(int U=L;U<=N;U++)for(int B=L;B<=U;B++)if(B>0&&B<N){int b=B<N-B?B:N-B;RowRanges[L][U][2]|=1u<<b;RowRanges[L][U][B&1]|=1u<<b;}
 }
 for(int idx=0;idx<5;idx++)if(ops&(1u<<idx)){
  int f=PopOps[idx],flip0=f==4||f==14,flip1=f==2||f==14,a0=0,b0=0,u=0,v=0,w=0,valid=1;
  /* Four bitsets encode, for every row simultaneously, allowed pairs00,10,01,11. */
  unsigned full=(1u<<N)-1u,m0=(~truth)&~(o[0]|o[1])&full;
  unsigned m1=((f&2)?truth:~truth)&~(z[0]|o[1])&full;
  unsigned m2=((f&4)?truth:~truth)&~(o[0]|z[1])&full;
  unsigned m3=((f&8)?truth:~truth)&~(z[0]|z[1])&full;
  if(flip0){unsigned q=m0;m0=m1;m1=q;q=m2;m2=m3;m3=q;}
  if(flip1){unsigned q=m0;m0=m2;m2=q;q=m1;m1=m3;m3=q;}
  if((m0|m1|m2|m3)!=full)valid=0;
  else{
   unsigned only11=m3&~(m0|m1|m2);
   a0=__builtin_popcount((m1&~(m0|m2|m3))|only11);
   b0=__builtin_popcount((m2&~(m0|m1|m3))|only11);
   if(f==6){u=__builtin_popcount(m0&m3);v=__builtin_popcount(m1&m2);}
   else{u=__builtin_popcount(m0&m1&~m2);v=__builtin_popcount(m0&m2&~m1);w=__builtin_popcount(m0&m1&m2);}
  }
  if(!valid)continue;
  for(int A=1;A<N;A++){
   int minority=A<N-A?A:N-A;if(!(d[0]&(1u<<minority)))continue;
   int a=(flip0?N-A:A)-a0,L,U,parity=2;
   if(f==6){int lo=a-v>0?a-v:0,hi=a<u?a:u;if(lo>hi)continue;L=b0+v-a+2*lo;U=b0+v-a+2*hi;parity=L&1;}
   else {if(a<0||a>u+w)continue;L=b0;U=b0+v+w-(a>u?a-u:0);if(flip1){int q=N-U;U=N-L;L=q;}}
   assert(L>=0&&L<=U&&U<=N);unsigned bs=RowRanges[L][U][parity]&d[1];
   if(bs){plain[0]|=1u<<minority;plain[1]|=bs;}
   if(1){int avail=budget-(charged0?2*minority*(N-minority):0);if(avail<0)bs=0;else if(charged1)bs&=RowCostMasks[avail>128?128:avail];}
   if(bs){support[0]|=1u<<minority;support[1]|=bs;}
  }
 }
}


int p9_joint_support(unsigned rows,unsigned target,unsigned operators,
 const unsigned zeros[2],const unsigned ones[2],const unsigned domains[2],
 int budget,unsigned charged,P9JointResult *result){
 if(!result||!zeros||!ones||!domains||(rows!=4&&rows!=8&&rows!=16)||operators>31||charged>3)return -1;
 unsigned full=(1u<<rows)-1u,mask=((1u<<(rows/2+1))-1u)&~1u;
 if((target&~full)||(target&1u))return -1;
 unsigned z[2],o[2],d[2];
 for(int i=0;i<2;i++){
  if(((zeros[i]|ones[i])&~full)||(zeros[i]&ones[i])||!(zeros[i]&1u)||(domains[i]&~mask))return -1;
  z[i]=zeros[i];o[i]=ones[i];d[i]=domains[i];
 }
 memset(result,0,sizeof(*result));
 P9ClosedSupport((int)rows,target,operators,z,o,d,result->supported,result->without_budget,budget<0?-1:budget,charged&1,(charged>>1)&1);
 return 0;
}
int p9_connection_feasible(unsigned rows,unsigned target,unsigned operators,
 const unsigned zeros[2],const unsigned ones[2],const unsigned domains[2],
 int budget,unsigned charged){
 P9JointResult result;int rc=p9_joint_support(rows,target,operators,zeros,ones,domains,budget,charged,&result);
 return rc?rc:(result.supported[0]!=0 && result.supported[1]!=0);
}
