#include <Settings.h>

#if SUBPROGRAM_EXECUTE == PIANO_ROLL_PREDICTION

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <fstream>
#include <sstream>

#include <unordered_set>
#include <unordered_map>

#include <sc/HTSL.h>

struct Frame {
	std::vector<unsigned short> _notes;
};

struct Sequence {
	int _seq;
	std::vector<Frame> _frames;
};

struct Dataset {
	std::vector<Sequence> _sequences;
};

void loadDataset(const std::string &fileName, Dataset &dataset) {
	std::ifstream ff(fileName);

	while (ff.good() && !ff.eof()) {
		std::string line;

		std::getline(ff, line);

		if (line == "")
			break;

		std::istringstream fl(line);

		std::string param;

		fl >> param;
		fl >> param;

		int seq = std::stoi(param.substr(0, param.length() - 1));

		fl >> param;

		int len;

		fl >> len;

		Sequence s;

		s._seq = seq;

		s._frames.resize(len);

		for (int f = 0; f < len; f++) {
			std::getline(ff, line);

			std::istringstream fnl(line);

			while (fnl.good() && !fnl.eof()) {
				std::string noteString;
				fnl >> noteString;

				if (noteString == "" || noteString == "n")
					break;

				s._frames[f]._notes.push_back(std::stoi(noteString));
			}
		}

		dataset._sequences.push_back(s);
	}
}

int main() {
	std::mt19937 generator(time(nullptr));

	Dataset train;

	loadDataset("resources/datasets/pianorolls/piano_rolls1.txt", train);

	const int useSequence = 0;

	std::unordered_set<int> usedNotes;

	for (int f = 0; f < train._sequences[useSequence]._frames.size(); f++) {
		Frame &frame = train._sequences[useSequence]._frames[f];

		for (int n = 0; n < frame._notes.size(); n++)
			if (usedNotes.find(frame._notes[n]) == usedNotes.end())
				usedNotes.insert(frame._notes[n]);
	}

	std::unordered_map<int, int> noteToInput;

	int count = 0;

	for (std::unordered_set<int>::const_iterator cit = usedNotes.begin(); cit != usedNotes.end(); cit++)
		noteToInput[*cit] = count++;

	sc::HTSL htsl;

	int squareDim = std::ceil(std::sqrt(static_cast<float>(usedNotes.size())));

	std::vector<sc::HTSL::LayerDesc> layerDescs(3);

	layerDescs[0]._width = 32;
	layerDescs[0]._height = 32;

	layerDescs[1]._width = 24;
	layerDescs[1]._height = 24;

	layerDescs[2]._width = 16;
	layerDescs[2]._height = 16;

	htsl.createRandom(squareDim, squareDim, layerDescs, generator);

	for (int loop = 0; loop < 5; loop++) {
		for (int f = 0; f < train._sequences[useSequence]._frames.size(); f++) {
			Frame &frame = train._sequences[useSequence]._frames[f];

			for (int i = 0; i < usedNotes.size(); i++)
				htsl.setInput(i, 0.0f);

			for (int n = 0; n < frame._notes.size(); n++)
				htsl.setInput(noteToInput[frame._notes[n]], 1.0f);
			
			htsl.update();
			htsl.learnRSC();
			htsl.learnPrediction();
			htsl.stepEnd();
		}
	}

	return 0;
}

#endif