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
	printf("Running PQC signature test\n");
	
	OQS_SIG *sig = OQS_SIG_new(OQS_SIG_alg_falcon_512);

	if (sig == NULL) {
		printf("Signature init failed");
		return 1;
	}
	
	uint8_t *public_key = malloc(sig->length_public_key);
	uint8_t *secret_key = malloc(sig->length_secret_key);
	uint8_t *signature = malloc(sig->length_signature);

	uint8_t message[] = "Hello from PQC";
	size_t message_len = strlen((char *)message);

	size_t signature_len;

	double start, end;

	//Key Generation
	start = get_time();
	sig->keypair(public_key, secret_key);
	end = get_time();
	printf("KeyGen time: %f seconds\n", end-start);
	

	//Signing
	start = get_time();
	sig->sign(signature, &signature_len, message, message_len, secret_key);
	end = get_time();
	printf("Sign time:%f seconds\n", end-start);

	//Verification
	start = get_time();
	int result = sig->verify(message,message_len,signature, signature_len,public_key);
	end = get_time();
	printf("Verify time: %f seconds\n",end-start);

	if (result == OQS_SUCCESS) {
		printf("Signature verified successfully\n");
	} else {
		printf("Signature verification failed\n");
	}

	free(public_key);
	free(secret_key);
	free(signature);
	OQS_SIG_free(sig);

	return 0;
}
