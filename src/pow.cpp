// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The DASH developers
// Copyright (c) 2015-2017 The PIVX developers
// Copyright (c) 2018 The TALUS developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "chain.h"
#include "chainparams.h"
#include "main.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

#include <math.h>


unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock)
{
    /* current difficulty formula, tal - DarkGravity v3, written by Evan Duffield - evan@dashpay.io */
    const CBlockIndex* BlockLastSolved = pindexLast;
    const CBlockIndex* BlockReading = pindexLast;
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = 24;
    int64_t PastBlocksMax = 24;
    int64_t CountBlocks = 0;
    uint256 PastDifficultyAverage;
    uint256 PastDifficultyAveragePrev;

    if (BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMin) {
        return Params().ProofOfWorkLimit().GetCompact();
    }
	if(pindexLast->nHeight >= 4 && pindexLast->nHeight <= Params().LAST_POW_BLOCK()) {
        uint256 bnNew;
		bnNew = ~uint256(0) >> 20;
		return bnNew.GetCompact();
	}
	// Proof of Stake
    if (pindexLast->nHeight > Params().LAST_POW_BLOCK()) {
        uint256 bnTargetLimit = Params().ProofOfWorkLimit();
		if (pindexLast->nHeight <= Params().LAST_POW_BLOCK() + 3 )
			return bnTargetLimit.GetCompact();
		
        int64_t nTargetSpacing = Params().TargetSpacing();
        int64_t nTargetTimespan = Params().TargetTimespan()*40;
 
        int64_t nActualSpacing = 0;
        if (pindexLast->nHeight != 0)
            nActualSpacing = pindexLast->GetBlockTime() - pindexLast->pprev->GetBlockTime();

        if (nActualSpacing < 0)
            nActualSpacing = 1;

        uint256 bnNew;
        bnNew.SetCompact(pindexLast->nBits);
		
        int64_t nInterval = nTargetTimespan / nTargetSpacing;
        bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
        bnNew /= ((nInterval + 1) * nTargetSpacing);
		
        if (bnNew <= 0 || bnNew > bnTargetLimit)
            bnNew = bnTargetLimit;
        return bnNew.GetCompact();
	}
	
    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) {
            break;
        }
        CountBlocks++;

        if (CountBlocks <= PastBlocksMin) {
            if (CountBlocks == 1) {
                PastDifficultyAverage.SetCompact(BlockReading->nBits);
            } else {
                PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) + (uint256().SetCompact(BlockReading->nBits))) / (CountBlocks + 1);
            }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if (LastBlockTime > 0) {
            int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
            nActualTimespan += Diff;
        }
        LastBlockTime = BlockReading->GetBlockTime();

        if (BlockReading->pprev == NULL) {
            assert(BlockReading);
            break;
        }
        BlockReading = BlockReading->pprev;
    }

    uint256 bnNew(PastDifficultyAverage);

    int64_t _nTargetTimespan = CountBlocks * Params().TargetSpacing();

    if (nActualTimespan < _nTargetTimespan / 3)
        nActualTimespan = _nTargetTimespan / 3;
    if (nActualTimespan > _nTargetTimespan * 3)
        nActualTimespan = _nTargetTimespan * 3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= _nTargetTimespan;
	
	
    if (bnNew > Params().ProofOfWorkLimit()) {
        bnNew = Params().ProofOfWorkLimit();
    }

	LogPrintf("GetNextWorkRequired RETARGET\n");
    LogPrintf("nActualTimespan = %d\n", nActualTimespan);
    LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());
	
    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
    bool fNegative;
    bool fOverflow;
    uint256 bnTarget;

    if (Params().SkipProofOfWorkCheck())
        return true;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > Params().ProofOfWorkLimit())
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    if (hash > bnTarget)
		return error("CheckProofOfWork() : hash doesn't match nBits");
	
    return true;
}

uint256 GetBlockProof(const CBlockIndex& block)
{
    uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    return (~bnTarget / (bnTarget + 1)) + 1;
}
