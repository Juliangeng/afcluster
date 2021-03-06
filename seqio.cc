#include "seqio.hh"
#include <ctype.h>

// ===== AUXILIARY FUNCTIONS =====

// Remove all whitespace from string in-place
static void removeWhiteSpace(string& line)
{
	size_t pos = 0;
	while (pos < line.length()) {
		if (isspace(line[pos])) {
			line.erase(pos, 1);
		} else {
			pos++;
		}
	}
	return;
}

// Extract accession number from the fasta header line
static string header2id(string line)
{
	// the starting position for the search is 1 (not 0) as the first 
	// character in the header line is ">", and we do not want it 
	// included in the accession number
	unsigned int bPos = line.find_first_not_of(" \t\r\n", 1); 
	unsigned int ePos = line.find_first_of(" \t\r\n", bPos);
	return line.substr(bPos, ePos + 1 - bPos);
}

// Reverse complement of a character
static inline char reverse_complement(char c)
{
	switch (c) {
	// uppercase characters
	case 'A':
		c = 'T';
		break;
	case 'C':
		c = 'G';
		break;
	case 'G':
		c = 'C';
		break;
	case 'T':
		c = 'A';
		break;
	case 'U': 
		c = 'A';
		break;
	// lowercase characters
	case 'a':
		c = 't';
		break;
	case 'c':
		c = 'g';
		break;
	case 'g':
		c = 'c';
		break;
	case 't':
		c = 'a';
		break;
	case 'u':
		c = 'a';
		break;
	// uppercase ambiguous characters
	case 'N':
		c = 'N';
		break;		
	case 'R': 
		c = 'Y';
		break;		
	case 'Y': 
		c = 'R';
		break;		
	case 'K':
		c= 'M';
		break;		
	case 'M': 
		c= 'K';
		break;		
	case 'S':
		c = 'S';
		break;		
	case 'W':
		c = 'W';
		break;		
	case 'B':
		c = 'V';
		break;		
	case 'V':
		c = 'B';
		break;		
	case 'D':
		c = 'H';
		break;		
	case 'H':
		c = 'D';
		break;		
	// lowercase ambiguous characters
	case 'n':
		c = 'n';
		break;
	case 'r': 
		c = 'y';
		break;
	case 'y': 
		c = 'r';
		break;
	case 'k':
		c= 'm';
		break;
	case 'm': 
		c= 'k';
		break;
	case 's':
		c = 's';
		break;
	case 'w':
		c = 'w';
		break;
	case 'b':
		c = 'v';
		break;
	case 'v':
		c = 'b';
		break;
	case 'd':
		c = 'h';
		break;
	case 'h':
		c = 'd';
		break;
	default:
		// should never get here
		c = c;
	}
	return c;
}

// Reverse complement of a sequence
static string reverse_complement(string seq)
{
	size_t L = seq.length();
	string rc_seq;
	rc_seq.resize(L);
	for(size_t i=0; i<L; i++) {
		rc_seq[i] = reverse_complement(seq[L-1-i]);
	}
	return rc_seq;
}


// ===== SEQRECORD CLASS IMPLEMENTATION =====

SeqRecord::SeqRecord(const SeqRecord &rec):
header(rec.id()), description(rec.desc()), sequence(rec.seq())
{}

SeqRecord::SeqRecord(string name, string desc, string seq):
header(name), description(desc), sequence(seq)
{}

SeqRecord::~SeqRecord()
{}

SeqRecord SeqRecord::toupper() const
{
	string upper_seq; 
	unsigned int L = sequence.length();
	upper_seq.resize(L);
	for (unsigned int i=0; i<L; i++) {
		upper_seq[i] = std::toupper(sequence[i]); // need to specify std
		// namespace explicitly to avoid conflict with our member function
	}
	return SeqRecord(header, description, upper_seq);
}

SeqRecord SeqRecord::rc() const
{
	return SeqRecord(header, description, reverse_complement(sequence));
}


// ===== RECORDGENERATOR CLASS IMPLEMENTATION =====

RecordGenerator::RecordGenerator(string fname):
ifs(fname.c_str()), id(""), desc(""), is_good(false)
{
	string line = "";
	while (ifs.good() && (!line.length() || line[0] != '>')) {
		getline(ifs, line);
	}
	if (line.length() && line[0] == '>') {
		id = header2id(line); 
		desc = line.substr(1); // initialize id and description now; sequence
		// is to be read in the call to the next() method
		is_good = true;
	}
}

RecordGenerator::~RecordGenerator()
{}

SeqRecord RecordGenerator::next()
{
	string line = "";
	string seq = "";
	while (ifs.good()) {
		getline(ifs, line);
		if (!line.length()) {
			continue;
		}
		if (line[0]!='>') {
			removeWhiteSpace(line);
			seq += line;
		} else {
			break;
		}
	}
	SeqRecord seq_rec(id, desc, seq);
	if (line[0] != '>') {
		is_good = false;
	}
	else {
		is_good = true;
		id = header2id(line); 
		desc = line.substr(1); // initialize for the next record to be read
	}
	return seq_rec;
}


// ===== FASTAREAD FUNCTION IMPLEMENTATION =====

void FastaRead(string fname, vector<SeqRecord>& recs)
{
	RecordGenerator rec_gen(fname);
	while (rec_gen.good()) {
		recs.push_back(rec_gen.next());
	}
	return;
}


// ===== COUNTREADS FUNCTION IMPLEMENTATION

int CountReads(string fname)
{
	int num_reads = 0;
	string line="";
	ifstream ifs(fname.c_str());
	while (ifs.good()) {
		getline(ifs, line);
		if (line[0] == '>') {
			num_reads++;
		}
	}
	return num_reads;
}
