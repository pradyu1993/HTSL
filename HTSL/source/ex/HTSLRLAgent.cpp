#include "HTSLRLAgent.h"

using namespace ex;

const int numPVContrastValues = 1;
const int numPVNodes = numPVContrastValues * 2;

const int numLVEContrastValues = 1;
const int numLVENodes = numLVEContrastValues * 2;

const int numLVIContrastValues = 1;
const int numLVINodes = numLVIContrastValues * 2;

void HTSLRLAgent::initialize(int numInputs, int numOutputs) {
	_generator.seed(time(nullptr));

	int rootDim = std::ceil(std::sqrt(static_cast<float>(numInputs * 2 + numOutputs * 2 + numPVNodes + numLVENodes + numLVINodes)));

	int totalState = numInputs * 2 + numOutputs + numPVContrastValues + numLVEContrastValues + numLVIContrastValues;

	std::vector<sc::HTSLRL::InputType> inputTypes(rootDim * rootDim);

	for (int i = 0; i < inputTypes.size(); i++) {
		if (i < totalState)
			inputTypes[i] = sc::HTSLRL::_state;
		else if (i < totalState + numOutputs)
			inputTypes[i] = sc::HTSLRL::_action;
		else if (i < totalState + numOutputs + numPVContrastValues)
			inputTypes[i] = sc::HTSLRL::_pv;
		else if (i < totalState + numOutputs + numPVContrastValues + numLVEContrastValues)
			inputTypes[i] = sc::HTSLRL::_lve;
		else if (i < totalState + numOutputs + numPVContrastValues + numLVEContrastValues + numLVIContrastValues)
			inputTypes[i] = sc::HTSLRL::_lvi;
		else 
			inputTypes[i] = sc::HTSLRL::_state;
	}

	std::vector<sc::HTSL::LayerDesc> layerDescs(3);

	layerDescs[0]._width = 32;
	layerDescs[0]._height = 32;

	layerDescs[1]._width = 24;
	layerDescs[1]._height = 24;

	layerDescs[2]._width = 16;
	layerDescs[2]._height = 16;

	_htslrl.createRandom(rootDim, rootDim, 8, inputTypes, layerDescs, _generator);
}

void HTSLRLAgent::getOutput(Experiment* pExperiment, const std::vector<float> &input, std::vector<float> &output, float reward, float dt) {
	int inputIndex = 0;
	
	for (int i = 0; i < input.size(); i++) {
		_htslrl.setState(inputIndex++, input[i]);
		_htslrl.setState(inputIndex++, input[i] + 1.0f);
	}

	for (int i = 0; i < _htslrl.getNumActionNodes(); i++)
		_htslrl.setState(inputIndex++, _htslrl.getActionFromNodeIndex(i) + 1.0f);

	for (int i = 0; i < numPVContrastValues; i++)
		_htslrl.setState(inputIndex++, _htslrl.getPVFromNodeIndex(i) + 1.0f);

	for (int i = 0; i < numLVEContrastValues; i++)
		_htslrl.setState(inputIndex++, _htslrl.getLVEFromNodeIndex(i) + 1.0f);

	for (int i = 0; i < numLVIContrastValues; i++)
		_htslrl.setState(inputIndex++, _htslrl.getLVIFromNodeIndex(i) + 1.0f);

	_htslrl.update(reward, _generator);

	output.resize(_htslrl.getNumActionNodes());

	for (int i = 0; i < output.size(); i++)
		output[i] = _htslrl.getActionFromNodeIndex(i) * 2.0f - 1.0f;
}