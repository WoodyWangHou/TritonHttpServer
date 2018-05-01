#ifndef FRAMER_HPP
#define FRAMER_HPP

#include <vector>
#include <string>
#include <iostream>

class Framer {
public:
	void append(std::string chars);

	// Does this buffer contain at least one complete message?
	bool hasMessage() const;

	// Copy the first instruction in this buffer into the provided class
	std::string topMessage() const;

	// Pops off the first instruction from the buffer
	void popMessage();

	// prints the string to an ostream (useful for debugging)
	void printToStream(std::ostream& stream) const;

	//count valid message number
	int countMessage() const;

	~Framer(){
		_msgBuffer.clear();
	}

protected:
	// PUT ANY FIELDS/STATE HERE
	std::vector<char> _msgBuffer;
	const char LF = '\n';
	const char CR = '\r';
};

#endif // CALCFRAMER_HPP
