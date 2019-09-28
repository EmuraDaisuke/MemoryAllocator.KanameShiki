


namespace KanameShiki {



// Node

class alignas(csCacheLine) GlobalHeap::Node final {
	public:
		~Node() noexcept							{ OrderCut(); FreeCut(); }
		
		Node(List* pOrder)							:mpOrder(pOrder),mpFree(nullptr){ assert(!(to_t(this) & cmCacheLine)); OrderJoinHead(); }
		Node(List* pOrder, Node* pJoin)				:mpOrder(pOrder),mpFree(nullptr){ assert(!(to_t(this) & cmCacheLine)); OrderJoinNode(pJoin); }
		
		bool IsFree() const noexcept				{ return (mpFree); }
		
		void Size(std::size_t s) noexcept			{ ms = s; }
		std::size_t Size() const noexcept			{ return ms; }
		
		Node* Prev() const noexcept					{ return mOrder.pPrev; }
		Node* Next() const noexcept					{ return mOrder.pNext; }
		Node* Exam() const noexcept					{ return mFree.pNext; }
		
		void Free(List* pFree) noexcept				{ FreeCut(); FreeJoinHead(pFree); }
		Node* Alloc() noexcept						{ FreeCut(); return this; }
		
		void* CastData() const noexcept				{ return offset_p(this, SizeofT()); }
		
		static Node* CastNode(void* p) noexcept		{ return offset_p<Node*>(p, -SizeofT()); }
		
		static constexpr std::size_t SizeofT() noexcept		{ return sizeof(Node); }
	
	private:
		void OrderCut() noexcept
		{
			Auto pPrev = mOrder.pPrev;
			Auto pNext = mOrder.pNext;
			
			if (mpOrder->pHead == this) mpOrder->pHead = pNext;
			if (pPrev) pPrev->mOrder.pNext = pNext;
			if (pNext) pNext->mOrder.pPrev = pPrev;
			
			mpOrder = nullptr;
		}
		
		
		
		void OrderJoinHead() noexcept
		{
			mpOrder->pHead = this;
			mOrder.pPrev = nullptr;
			mOrder.pNext = nullptr;
		}
		
		
		
		void OrderJoinNode(Node* pJoin) noexcept
		{
			Auto pNext = pJoin->mOrder.pNext;
			
			pJoin->mOrder.pNext = this;
			if (pNext) pNext->mOrder.pPrev = this;
			mOrder.pPrev = pJoin;
			mOrder.pNext = pNext;
		}
		
		
		
		void FreeCut() noexcept
		{
			if (mpFree){
				Auto pPrev = mFree.pPrev;
				Auto pNext = mFree.pNext;
				
				if (mpFree->pHead == this) mpFree->pHead = pNext;
				if (pPrev) pPrev->mFree.pNext = pNext;
				if (pNext) pNext->mFree.pPrev = pPrev;
				
				mpFree = nullptr;
			}
		}
		
		
		
		void FreeJoinHead(List* pFree) noexcept
		{
			Auto pNext = pFree->pHead;
			
			pFree->pHead = this;
			if (pNext) pNext->mFree.pPrev = this;
			mFree.pPrev = nullptr;
			mFree.pNext = pNext;
			
			mpFree = pFree;
		}
	
	
	private:
		struct Chain {
			Node* pPrev;
			Node* pNext;
		};
		
		List* mpOrder;
		List* mpFree;
		Chain mOrder;
		Chain mFree;
		std::size_t ms;
};



// GlobalHeap

GlobalHeap::GlobalHeap(bool bInit)
:ms(csGlobalHeap)
,mp(SystemAlloc(ms))
,mnAlloc(0)
,mOrder{}
,maFree{}
,mabFree(0)
{
	assert(mp);
	
	{	// 
		Auto sNode = Node::SizeofT();
		Auto sFree = bound_t(ms - sNode, csCacheLine);
		
		Auto pFree = reinterpret_cast<Node*>(mp);
		new(pFree) Node(&mOrder);
		
		AddFree(pFree, sFree);
	}
}



void GlobalHeap::Destruct() noexcept
{
	SystemFree(mp, ms);
	
	#if KANAMESHIKI_DEBUG_LEVEL == 2//[
	assert(mnAlloc == 0);
	#endif//]
}



void GlobalHeap::Free(void* p) noexcept
{
	Auto pNode = Node::CastNode(p);
	Auto sFree = pNode->Size();
	
	{	// 
		std::lock_guard<std::mutex> Lock(mMutex);
		
		Auto pNext = pNode->Next();
		if (pNext && pNext->IsFree()){
			Auto sNext = pNext->Size();
			Auto oNext = Lower(sNext);
			
			sFree += Node::SizeofT();
			sFree += sNext;
			
			pNext->~Node();
			FreeOff(oNext);
		}
		
		Auto pPrev = pNode->Prev();
		if (pPrev && pPrev->IsFree()){
			Auto sPrev = pPrev->Size();
			Auto oPrev = Lower(sPrev);
			
			sFree += Node::SizeofT();
			sFree += sPrev;
			
			AddFree(pPrev, sFree);
			
			pNode->~Node();
			FreeOff(oPrev);
		} else {
			AddFree(pNode, sFree);
		}
		
		#if KANAMESHIKI_DEBUG_LEVEL == 2//[
		--mnAlloc;
		#endif//]
	}
}



void* GlobalHeap::AllocUpper(std::size_t s) noexcept
{
	s = align_t(s, csCacheLine);
	Auto b = Upper(s);
	Auto m = -bit(b);
	
	{	// 
		std::lock_guard<std::mutex> Lock(mMutex);
		
		Auto abFree = mabFree & m;
		if (abFree){
			Auto oFree = Lzc::Lsb(abFree);
			Auto pFree = maFree[oFree].pHead;
			return Alloc(pFree, oFree, s);
		}
	}
	return nullptr;
}



void* GlobalHeap::AllocLower(std::size_t s) noexcept
{
	s = align_t(s, csCacheLine);
	Auto b = Lower(s);
	Auto m = -bit(b);
	
	{	// 
		std::lock_guard<std::mutex> Lock(mMutex);
		
		Auto abFree = mabFree & m;
		if (abFree){
			Auto oFree = Lzc::Lsb(abFree);
			Auto pFree = maFree[oFree].pHead;
			for (; pFree; pFree = pFree->Exam()){
				if (pFree->Size() >= s){
					return Alloc(pFree, oFree, s);
				}
			}
		}
	}
	return nullptr;
}



void GlobalHeap::AddFree(Node* pFree, std::size_t s) noexcept
{
	Auto oFree = Lower(s);
	pFree->Free(&maFree[oFree]);
	pFree->Size(s);
	FreeOn(oFree);
}



void* GlobalHeap::Alloc(Node* pFree, uint8_t oFree, std::size_t s) noexcept
{
	Auto pAlloc = pFree->Alloc();
	Auto p = pAlloc->CastData();
	FreeOff(oFree);
	
	Auto sAlloc = pAlloc->Size();
	Auto sRest = sAlloc - s;
	if (sRest >= Node::SizeofT()){
		sRest -= Node::SizeofT();
		
		pAlloc->Size(s);
		
		Auto pRest = offset_p<Node*>(p, s);
		new(pRest) Node(&mOrder, pAlloc);
		
		AddFree(pRest, sRest);
	}
	
	#if KANAMESHIKI_DEBUG_LEVEL == 2//[
	++mnAlloc;
	#endif//]
	return p;
}



void GlobalHeap::FreeOn(uint8_t oFree) noexcept
{
	mabFree |= bit(oFree);
}



void GlobalHeap::FreeOff(uint8_t oFree) noexcept
{
	if (!maFree[oFree].pHead) mabFree &= ~bit(oFree);
}



uint8_t GlobalHeap::Lower(std::size_t s) noexcept
{
	return Lzc::Msb(s)+1;
}



uint8_t GlobalHeap::Upper(std::size_t s) noexcept
{
	return (s)? Lzc::Msb(s+s-1)+1:0;
}



}
