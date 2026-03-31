#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <oqs/oqs.h>

double get_time() {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + t.tv_nsec * 1e-9;
}

int main() {
	printf("Running PQC KEM test\n");
	
	OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_768);

	if (kem == NULL) {
		printf("KEM initialization failed/n");
		return 1;
	}

	uint8_t *public_key = malloc(kem->length_public_key);
	uint8_t *secret_key = malloc(kem->length_secret_key);
	uint8_t *ciphertext = malloc(kem->length_ciphertext);
	uint8_t *shared_secret1 = malloc(kem->length_shared_secret);
	uint8_t *shared_secret2 = malloc(kem->length_shared_secret);

	double start;
	double end;

	//Key Generation Test

	start = get_time();
	kem->keypair(public_key, secret_key);
	end = get_time();

	printf("KeyGen time: %f seconds\n", end - start);

	//Encapsulation Test
	
	start = get_time();
	kem->encaps(ciphertext, shared_secret1, public_key);
	end = get_time();

	printf("Encapsulation time: %f seconds\n", end - start);

	//Decapsulation Test

	start = get_time();
	kem->decaps(shared_secret2, ciphertext, secret_key);
	end = get_time();

	printf("Decapsulation time: %f seconds\n", end - start);

	//Verify Result

	if (memcmp(shared_secret1, shared_secret2, kem->length_shared_secret) == 0) {
		printf("Shared secret verified\n");
	} else {
		printf("Shared secret mismatch\n");
	}

	free(public_key);
	free(secret_key);
	free(ciphertext);
	free(shared_secret1);
	free(shared_secret2);

	OQS_KEM_free(kem);

	return 0;

		
}

