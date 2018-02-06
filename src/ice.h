/*
 * Header file for the ICE encryption library.
 *
 * Written by Matthew Kwan - July 1996
 */

#ifndef _ICE_H
#define _ICE_H

typedef struct ice_key_struct	ICE_KEY;

extern ICE_KEY	*ice_key_create(int n);
extern void	ice_key_destroy(ICE_KEY *ik);
extern void	ice_key_set(ICE_KEY *ik, const unsigned char *k);
extern void	ice_key_encrypt(const ICE_KEY *ik,
			const unsigned char *ptxt, unsigned char *ctxt);
extern void	ice_key_decrypt(const ICE_KEY *ik,
			const unsigned char *ctxt, unsigned char *ptxt);
extern int	ice_key_key_size(const ICE_KEY *ik);
extern int	ice_key_block_size(const ICE_KEY *ik);

#endif
