unsigned long hash_function(unsigned char *str) {
	unsigned long hash = 13; // some prime

	for(int i = 0; str[i] != '0'; ++i) {
		hash = 23 * hash + (int)str[i];
	}

	return hash;
}
