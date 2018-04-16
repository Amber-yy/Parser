#include "AStree.h"
#include "Parser.h"

Parser * AStree::parser;

AStree::AStree()
{
	block = nullptr;
}

AStree::~AStree()
{

}

bool AStree::isBlock()
{
	return false;
}

Block * AStree::toBlock()
{
	return dynamic_cast<Block *>(this);
}

Result Block::eval()
{
	return Result();
}

bool Block::isLeftValue()
{
	return false;
}

bool Block::isBlock()
{
	return true;
}
