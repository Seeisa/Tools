class CRC32 : public HashBase
{
public:
	//=========================================
	//  ctors
	inline CRC32() { Reset(); }
	inline CRC32(const void* buf, unsigned int siz) { Reset(); Hash(buf, siz); }

	//=========================================
	// implicit cast, so that you can do something like foo = CRC(dat,siz);
	inline operator u32 () const { return Get(); }

	//=========================================
	// getting the crc
	inline u32          Get() const { return ~mCrc; }

	//=========================================
	// HashBase stuff
	virtual void        Reset() { mCrc = ~0; }
	virtual void        Hash(const void* buf, unsigned int siz);

private:
	u32         mCrc;
	static bool mTableBuilt;
	static u32  mTable[0x100];

	static const u32        POLYNOMIAL = 0x04C11DB7;

private:
	//=========================================
	// internal support
	static void         BuildTable();
	static u32          Reflect(u32 v, int bits);
};

#pragma once
