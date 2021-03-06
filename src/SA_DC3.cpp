/*
 * DC3.cpp
 * Difference Cover (mod 3) suffix array construction algorithm
 * Base algorithm copied from Karkkainen 2006 -
 * 	Linear Work Suffix Array Construction (Appendix A)
 * Has been modified to work with Byte input.
 */
#include "SA_DC3.h"

#include <iostream>

// Convert input string into integers.
vector<int> createIntVector(vector<BYTE> inputString) {
	vector<int> output (inputString.size() + 3, 0);
	for (vector<int>::size_type i = 0; i < inputString.size(); ++i) {
		output[i] = inputString[i];
	}
	return output;
}


// lexicographic order for pairs and triples

inline bool leq(int a1, int a2, int b1, int b2) {
	return (a1 < b1 || (a1 == b1 && a2 < b2));
}

inline bool leq(int a1, int a2, int a3, int b1, int b2, int b3) {
	return (a1 < b1 || (a1 == b1 && leq(a2, a3, b2, b3)));
}

// stably sort a[0..n-1] to b[0..n-1] with keys in 0..K from r
static void radixPass(int* a, int* b, int* r, int n, int K) {
	int* c = new int[K + 1];
	for (int i = 0; i <= K; i++) c[i] = 0;
	for (int i = 0; i < n; i++) c[r[a[i]]]++;
	for (int i = 0, sum = 0; i <= K; i++) {
		int t = c[i]; c[i] = sum; sum += t;
	}
	for (int i = 0; i < n; i++) b[c[r[a[i]]]++] = a[i];
	delete [] c;
}


// Find the suffix array SA of T[0..n-1] in {1..K}^n
// 'n' length of input - note this is 3 smaller than array length
// 'K' is size of dictionary of characters in input array
// Require T[n]=T[n+1]=T[n+2]=0, n>=2
void buildSuffixArray(int* inputInts, int* SA, int n, int K) {
	int n0=(n+2)/3, n1=(n+1)/3, n2=n/3, n02=n0+n2;
	int* R = new int[n02 + 3];
	R[n02] = R[n02+1] = R[n02+2] = 0;
	int* SA12 = new int[n02 + 3];
	SA12[n02] = SA12[n02+1] = SA12[n02+2]=0;
	int* R0 = new int[n0];
	int* SA0 = new int[n0];
	//******* Step 0: Construct sample ********
	// generate positions of mod 1 and mod 2 suffixes
	// the "+(n0-n1)" adds a dummy mod 1 suffix if n%3 == 1
	for (int i=0, j=0; i < n+(n0-n1); i++) {
		if (i%3 != 0) R[j++] = i;
	}

	//******* Step 1: Sort sample suffixes ********
	// lsb radix sort the mod 1 and mod 2 triples
	radixPass(R , SA12, inputInts+2, n02, K);
	radixPass(SA12, R , inputInts+1, n02, K);
	radixPass(R , SA12, inputInts , n02, K);
	// find lexicographic names of triples and
	// write them to correct places in R
	int name = 0, c0 = -1, c1 = -1, c2 = -1;
	for (int i = 0; i < n02; i++) {
		if (inputInts[SA12[i]] != c0 ||
			inputInts[SA12[i]+1] != c1 ||
			inputInts[SA12[i]+2] != c2)
		{
			name++; c0 = inputInts[SA12[i]]; c1 = inputInts[SA12[i]+1]; c2 = inputInts[SA12[i]+2];
		}
		if (SA12[i] % 3 == 1) {
			// write to R1
			R[SA12[i]/3] = name;
		} else {
			// write to R2
			R[SA12[i]/3 + n0] = name;
		}
	}

	// recurse if names are not yet unique
	if (name < n02) {
		buildSuffixArray(R, SA12, n02, name);
		// store unique names in R using the suffix array
		for (int i = 0; i < n02; i++) R[SA12[i]] =i+1;
	} else {
		// generate the suffix array of R directly
		for (int i = 0; i < n02; i++) SA12[R[i] - 1] = i;
	}
	//******* Step 2: Sort nonsample suffixes ********
	// stably sort the mod 0 suffixes from SA12 by their first character
	for (int i=0, j=0; i < n02; i++) if (SA12[i] < n0) R0[j++] = 3*SA12[i];
	radixPass(R0, SA0, inputInts, n0, K);
	//******* Step 3: Merge ********
	// merge sorted SA0 suffixes and sorted SA12 suffixes
	for (int p=0, t=n0-n1, k=0; k < n; k++) {
		#define GetI() (SA12[t] < n0 ? SA12[t] *3+1: (SA12[t] - n0) * 3 + 2)
		int i = GetI(); // pos of current offset 12 suffix
		int j = SA0[p]; // pos of current offset 0 suffix
		if (SA12[t] < n0 ? leq(inputInts[i],R[SA12[t] + n0], inputInts[j],R[j/3]): leq(inputInts[i],inputInts[i+1],R[SA12[t]-n0+1], inputInts[j],inputInts[j+1],R[j/3+n0])) {
			SA[k] = i; t++;
			if (t == n02) // done --- only SA0 suffixes left
			for (k++; p < n0; p++, k++) SA[k] = SA0[p];
		} else {
			// Suffix from SA0 is smaller
			SA[k] = j; p++;
			if (p == n0) // done --- only SA12 suffixes left
			for (k++; t < n02; t++, k++) SA[k] = GetI();
		}
	}
	delete [] R; delete [] SA12; delete [] SA0; delete [] R0;
}

vector<int> DC3(vector<int> &inputInts, int n, int K) {
	vector<int> SA (n + 3, 0);
	buildSuffixArray(inputInts.data(), SA.data(), n, K);
	// Remove the 3 padding zeroes at the end
	while ((int)SA.size() > n) SA.pop_back();
	return SA;
}

vector<int> DC3(int* inputInts, int n, int K) {
	vector<int> SA (n + 3, 0);
	buildSuffixArray(inputInts, SA.data(), n, K);
	while ((int)SA.size() > n) SA.pop_back();
	return SA;
}


