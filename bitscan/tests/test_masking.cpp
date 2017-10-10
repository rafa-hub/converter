//tests with masks for sparse and normal (non-sparse) graphs

#include <algorithm>
#include <iterator>
#include <iostream>
#include <set>

#include "../bitscan.h"					//bit string library
#include "gtest/gtest.h"
#include "utils/common.h"
#include <vector>

using namespace std;

TEST(Masks, AND_OR) {

//non sparse
	bitarray bb(130);
	bitarray bb1(130);
	bitarray bbresAND(130);
	bitarray bbresOR(130);
	
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);

	bb1.set_bit(10);
	bb1.set_bit(64);
	bb1.set_bit(100);

	//AND
	cout<<AND(bb, bb1, bbresAND)<<endl;
	EXPECT_TRUE(bbresAND.is_bit(10));
	EXPECT_TRUE(bbresAND.is_bit(64));
	EXPECT_EQ(2, bbresAND.popcn64());

	//OR
	cout<<OR(bb, bb1, bbresOR)<<endl;
	EXPECT_EQ(4, bbresOR.popcn64());

	//&=
	bitarray bb2(130);
	bitarray bb3(130);

	bb2.set_bit(10);
	bb2.set_bit(20);
	bb2.set_bit(64);

	bb3.set_bit(10);
	bb3.set_bit(64);
	bb3.set_bit(100);

	bb2&=bb3;
	EXPECT_TRUE(bbresAND.is_bit(10));
	EXPECT_TRUE(bbresAND.is_bit(64));
	EXPECT_EQ(2,bb2.popcn64());
	
}

TEST(Masks, AND_OR_sparse) {

//non sparse
	sparse_bitarray bb(130);
	sparse_bitarray bb1(130);
	sparse_bitarray bbresAND(130);
	sparse_bitarray bbresOR(130);
	
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);

	bb1.set_bit(10);
	bb1.set_bit(64);
	bb1.set_bit(100);

	//AND
	cout<<AND(bb, bb1, bbresAND)<<endl;
	EXPECT_TRUE(bbresAND.is_bit(10));
	EXPECT_TRUE(bbresAND.is_bit(64));
	EXPECT_EQ(2, bbresAND.popcn64());

	//OR
	cout<<OR(bb, bb1, bbresOR)<<endl;
	EXPECT_EQ(4, bbresOR.popcn64());

	//&=
	sparse_bitarray bb2(130);
	sparse_bitarray bb3(130);

	bb2.set_bit(10);
	bb2.set_bit(20);
	bb2.set_bit(64);

	bb3.set_bit(10);
	bb3.set_bit(64);
	bb3.set_bit(100);

	bb2&=bb3;
	EXPECT_TRUE(bb2.is_bit(10));
	EXPECT_TRUE(bb2.is_bit(64));
	EXPECT_EQ(2,bb2.popcn64());

}


TEST(Masks, set_bits){

	bitarray bb(130);
	bitarray bb1(130);
	bitarray bbres(130);
	
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);

	bb1.set_bit(10);
	bb1.set_bit(64);
	bb1.set_bit(100);
	
	//set range
	bb.set_block(0,bb1);
	EXPECT_TRUE(bb.is_bit(100));
	EXPECT_TRUE(bb.is_bit(20));
		
	bb1.set_bit(129);
	bb.set_block(0,1,bb1);
	EXPECT_FALSE(bb.is_bit(129));
		
	bb.set_block(0,2,bb1);
	EXPECT_TRUE(bb.is_bit(129));

	//erase range
	bb.erase_block(2, bb1);
	EXPECT_FALSE(bb.is_bit(129));

	bb.erase_block(1, bb1);
	EXPECT_FALSE(bb.is_bit(100));
	EXPECT_FALSE(bb.is_bit(64));

	cout<<bb.erase_block(0, bb1)<<endl;
	EXPECT_TRUE(bb.is_bit(20));
}

TEST(Masks, set_bits_sparse){	
	sparse_bitarray bb(130);
	sparse_bitarray bb1(130);
	sparse_bitarray bbres(130);
	
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);

	bb1.set_bit(10);
	bb1.set_bit(64);
	bb1.set_bit(100);
	
	//set range
	bb.set_block(0,bb1);
	EXPECT_TRUE(bb.is_bit(100));
	
	bb1.set_bit(129);
	bb.set_block(0,1,bb1);
	EXPECT_FALSE(bb.is_bit(129));

	bb.set_block(0,2,bb1);
	EXPECT_TRUE(bb.is_bit(129));

	//erase range
	cout<<bb<<endl;
	bb.erase_block(2, bb1);
	EXPECT_FALSE(bb.is_bit(129));

	bb.erase_block(1, bb1);
	EXPECT_FALSE(bb.is_bit(100));
	EXPECT_FALSE(bb.is_bit(64));

	cout<<bb.erase_block(0, bb1)<<endl;
	EXPECT_TRUE(bb.is_bit(20));
}

TEST(Masks, erase_bits){
	bitarray bb(130);
	bitarray bb1(130);
	bitarray bbres(130);
	
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);

	bb1.set_bit(10);
	bb1.set_bit(64);
	bb1.set_bit(100);

	bb.erase_block(2,2,bb1);		//nothing deleted
	EXPECT_EQ(3, bb.popcn64());

	bb.erase_block(1,1,bb1);		//nothing deleted
	EXPECT_TRUE(bb.is_bit(10));
	EXPECT_FALSE(bb.is_bit(64));
	EXPECT_TRUE(bb.is_bit(20));
	EXPECT_FALSE(bb.is_bit(100));

	bb.erase_block(0,2,bb1);		//nothing deleted
	EXPECT_EQ(1, bb.popcn64());
}

TEST(Masks, erase_bits_sparse){
	sparse_bitarray bb(130);
	sparse_bitarray bb1(130);
	sparse_bitarray bbres(130);
	
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);

	bb1.set_bit(10);
	bb1.set_bit(64);
	bb1.set_bit(100);

	bb.erase_block(2,2,bb1);		//nothing deleted
	EXPECT_EQ(3, bb.popcn64());

	bb.erase_block(1,1,bb1);		//nothing deleted
	EXPECT_TRUE(bb.is_bit(10));
	EXPECT_FALSE(bb.is_bit(64));
	EXPECT_TRUE(bb.is_bit(20));
	EXPECT_FALSE(bb.is_bit(100));

	bb.erase_block(0,2,bb1);		//nothing deleted
	EXPECT_EQ(1, bb.popcn64());
}

TEST(Masks, ERASE){					
///////////
// date: 17/12/15

//non-sparse
	bitarray bb(130);
	bitarray bb1(130);
	bitarray bbERASE(130);
	
	
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);

	bb1.set_bit(10);
	bb1.set_bit(64);
	bb1.set_bit(100);

	//ERASE
	cout<<ERASE(bb, bb1, bbERASE)<<endl;
	EXPECT_TRUE(bbERASE.is_bit(20));
	EXPECT_FALSE(bbERASE.is_bit(10));
	EXPECT_FALSE(bbERASE.is_bit(64));
	EXPECT_EQ(1, bbERASE.popcn64());

//sparse
	sparse_bitarray bbs(130);
	sparse_bitarray bbs1(130);
	sparse_bitarray bbsERASE(130);
	
	
	bbs.set_bit(10);
	bbs.set_bit(20);
	bbs.set_bit(64);

	bbs1.set_bit(10);
	bbs1.set_bit(64);
	bbs1.set_bit(100);

	//ERASE
	cout<<ERASE(bbs, bbs1, bbsERASE)<<endl;
	EXPECT_TRUE(bbsERASE.is_bit(20));
	EXPECT_FALSE(bbsERASE.is_bit(10));
	EXPECT_FALSE(bbsERASE.is_bit(64));
	EXPECT_EQ(1, bbsERASE.popcn64());
}

TEST(Masks, ERASE_extreme_cases){
///////////
// date: 21/12/15

//non-sparse
	bitarray bb(130);				
	bitarray bb1(130);				//empty
	bitarray bbERASE(130);
	bbERASE.set_bit(129);			
	
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);	
	
	//ERASE
	cout<<ERASE(bb, bb1, bbERASE)<<endl;
	EXPECT_TRUE(bb==bbERASE);

//sparse
	sparse_bitarray bbs(130);
	sparse_bitarray bbs1(130);			//empty
	sparse_bitarray bbsERASE(130);


	bbs.set_bit(10);
	bbs.set_bit(64);
	bbs.set_bit(100);

	
	//ERASE
	cout<<ERASE(bbs, bbs1, bbsERASE)<<endl;
	EXPECT_TRUE(bbs==bbsERASE);

//erase when no blocks in same index: simple copy

	bbs1.erase_bit();
	bbs1.set_bit(100);

	bbs.print();
	bbs1.print();

	//ERASE
	cout<<ERASE(bbs, bbs1, bbsERASE)<<endl;
	EXPECT_TRUE(bbsERASE.is_bit(10));
	EXPECT_TRUE(bbsERASE.is_bit(64));
	EXPECT_EQ(2, bbsERASE.popcn64());
}

TEST(Masks, AND_trimming){
///////////
//date: 22/6/16
// Note: AND works in a half open range (excludes limiting bit)

	bitarray bb(130);
	bitarray bb1(130);
	bitarray bbresAND(130);

	
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);

	bb1.set_bit(10);
	bb1.set_bit(64);
	bb1.set_bit(100);

	//AND
	cout<<AND(bb, bb1, bbresAND,11)<<endl;
	EXPECT_TRUE(bbresAND.is_bit(10));
	EXPECT_FALSE(bbresAND.is_bit(64));
	EXPECT_EQ(1, bbresAND.popcn64());

	bbresAND.erase_bit();
	cout<<AND(bb, bb1, bbresAND,10)<<endl;
	EXPECT_FALSE(bbresAND.is_bit(10));

	bbresAND.erase_bit();
	cout<<AND(bb, bb1, bbresAND,64)<<endl;
	EXPECT_FALSE(bbresAND.is_bit(64));

	bbresAND.erase_bit();
	cout<<AND(bb, bb1, bbresAND,65)<<endl;
	EXPECT_TRUE(bbresAND.is_bit(64));
		

	//AND
	bbresAND.erase_bit();
	cout<<AND(bb, bb1, bbresAND, 5)<<endl;
	EXPECT_FALSE(bbresAND.is_bit(10));
	EXPECT_FALSE(bbresAND.is_bit(64));
	EXPECT_TRUE(1, bbresAND.is_empty());
	
}

TEST(Masks, AND_trimming_2_vertex_set){
///////////
//date: 26/6/16
// Note: AND works in a half open range (excludes limiting bit)

	bitarray bb(130);
	bitarray bb1(130);
		
	bb.set_bit(10);
	bb.set_bit(20);
	bb.set_bit(64);

	bb1.set_bit(10);
	bb1.set_bit(64);
	bb1.set_bit(100);

	int v[130];
	int size=0;

	//AND
	AND(bb, bb1, 11, v, size);			//v[0]=10;
	vector<int> vset(v, v+size);
	EXPECT_TRUE(find(vset.begin(), vset.end(), 10)!=vset.end());
	EXPECT_FALSE(find(vset.begin(), vset.end(), 64)!=vset.end());
	EXPECT_EQ(1, size);
	
	AND(bb, bb1, 10, v, size);			//v=[];
	EXPECT_EQ(0, size);

	AND(bb, bb1, 65,v,size);			//v={10,64}
	vector<int> vset2(v, v+size);
	EXPECT_TRUE(find(vset2.begin(), vset2.end(), 64)!=vset2.end());

}