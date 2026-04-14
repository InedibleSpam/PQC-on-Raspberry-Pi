#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/ec.h>


double get_time() {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(){
	printf("Running Classical Crypto Test\n\n");
	
	double start, end;


	EVP_PKEY_CTX *pctx;
	EVP_PKEY *key1 = NULL;
	EVP_PKEY *key2 = NULL;

	pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
	EVP_PKEY_keygen_init(pctx);

	EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1);
	
	start = get_time();
	EVP_PKEY_keygen(pctx, &key1);
	end = get_time();
	printf("ECDH KeyGen time: %f seconds\n", end - start);
	//printf(key1);
	//printf(key2);
	EVP_PKEY_keygen(pctx, &key2);

	EVP_PKEY_CTX *derive_ctx;
	size_t secret_len;
	unsigned  char *secret;
	
	derive_ctx = EVP_PKEY_CTX_new(key1, NULL);
	EVP_PKEY_derive_init(derive_ctx);
	EVP_PKEY_derive_set_peer(derive_ctx, key2);

	start = get_time();
	EVP_PKEY_derive(derive_ctx, NULL, &secret_len);
	secret = malloc(secret_len);
	EVP_PKEY_derive(derive_ctx, secret, &secret_len);
	end = get_time();

	printf("ECDH Shared Secret Time: %f seconds\n", end - start);


	unsigned char message[] = "We were schooner-rigged and rakish,/n" 
"with a long and lissome hull,\n"
"And we flew the pretty colours of the crossbones and the skull;\n" 
"We'd a big black Jolly Roger flapping grimly at the fore, \n"
"And we sailed the Spanish Water in the happy days of yore. \n"
"We'd a long brass gun amidships, like a well-conducted ship,\n" 
"We had each a brace of pistols and a cutlass at the hip; \n"
"It's a point which tells against us, and a fact to be deplored, \n"
"But we chased the goodly merchant-men and laid their ships aboard. \n"
"Then the dead men fouled the scuppers and the wounded filled the chains,\n" 
"And the paint-work all was spatter dashed with other peoples brains,\n" 
"She was boarded, she was looted, she was scuttled till she sank.\n" 
"And the pale survivors left us by the medium of the plank. \n"
"O! then it was (while standing by the taffrail on the poop) \n"
"We could hear the drowning folk lament the absent chicken coop;\n" 
"Then, having washed the blood away, we'd little else to do \n"
"Than to dance a quiet hornpipe as the old salts taught us to. \n"
"O! the fiddle on the fo'c'sle, and the slapping naked soles, \n"
"And the genial Down the middle, Jake, and curtsey when she rolls!\n" 
"With the silver seas around us and the pale moon overhead, \n"
"And the look-out not a-looking and his pipe-bowl glowing red. \n"
"Ah! the pig-tailed, quidding pirates and the pretty pranks we played,\n" 
"All have since been put a stop to by the naughty Board of Trade;\n" 
"The schooners and the merry crews are laid away to rest,\n"
"A little south the sunset in the islands of the Blest.\n";
	size_t msg_len = strlen((char *)message);
	
	unsigned char signature[256];
	size_t sig_len;

	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

	start = get_time();
	EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, key1);
	EVP_DigestSign(mdctx, signature, &sig_len, message, msg_len);
	end = get_time();

	printf("ECDSA Sign time: %f seconds\n", end - start);

	start = get_time();
	EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, key1);
	EVP_DigestVerify(mdctx, signature, sig_len, message, msg_len);
	end = get_time();

	printf("ECDSA Verify time; %f seconds\n" , end - start);


	EVP_MD_CTX_free(mdctx);
	EVP_PKEY_free(key1);
	EVP_PKEY_free(key2);
	EVP_PKEY_CTX_free(pctx);
	EVP_PKEY_CTX_free(derive_ctx);
	free(secret);

	return 0;
}
