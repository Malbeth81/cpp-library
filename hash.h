/*
* Hash.h - An implementation of a super fast hash
*
* v 1.0
*
* Copyright (C) 2007-2010 Marc-André Lamothe.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Library General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifndef HASH_H_
#define HASH_H_
/*
inline unsigned int VerySimpleHash(const char* data, int len)
{
  const int p = 16777619;
  unsigned int hash = 2166136261U;

  for (int i = 0; i < len; i++)
    hash = (hash ^ data[i])*p;

  hash += hash << 13;
  hash ^= hash >> 7;
  hash += hash << 3;
  hash ^= hash >> 17;
  hash += hash << 5;
  return hash;
}
*/

/*
 * http://burtleburtle.net/bob/hash/doobs.html
 */

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
* If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
* If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
--------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  data       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i)
    h = hash(k[i], len[i], h);

By Bob Jenkins, 1996. bob_jenkins@burtleburtle.net. You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/
inline unsigned int ComputeHash(const char* data, unsigned int length, unsigned int initval = 0)
{
  /* Set up the internal state */
  unsigned int a = 0x9e3779b9;
  unsigned int b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
  unsigned int c = initval;

  /* handle most of the key */
  while (length >= 12)
  {
    a += (data[0] +((unsigned int)data[1]<<8)+((unsigned int)data[2]<<16)+((unsigned int)data[3]<<24));
    b += (data[4] +((unsigned int)data[5]<<8)+((unsigned int)data[6]<<16)+((unsigned int)data[7]<<24));
    c += (data[8] +((unsigned int)data[9]<<8)+((unsigned int)data[10]<<16)+((unsigned int)data[11]<<24));
    mix(a,b,c);
    data += 12;
    length -= 12;
  }

  /* handle the last 11 bytes */
  c += length;
  switch(length) /* all the case statements fall through */
  {
  case 11: c+=((unsigned int)data[10]<<24);
  case 10: c+=((unsigned int)data[9]<<16);
  case 9 : c+=((unsigned int)data[8]<<8); /* the first byte of c is reserved for the length */
  case 8 : b+=((unsigned int)data[7]<<24);
  case 7 : b+=((unsigned int)data[6]<<16);
  case 6 : b+=((unsigned int)data[5]<<8);
  case 5 : b+=data[4];
  case 4 : a+=((unsigned int)data[3]<<24);
  case 3 : a+=((unsigned int)data[2]<<16);
  case 2 : a+=((unsigned int)data[1]<<8);
  case 1 : a+=data[0];
  }
  mix(a,b,c);
  return c;
}

#endif
