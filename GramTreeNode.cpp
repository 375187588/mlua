#include "Scanner.h"
#include "GramTreeNode.h"
#include "Visitor.h"

int SyntaxTreeNodeBase::s_nCurNodeIndex = 0;

void SyntaxTreeNodeBase:: addChild(SyntaxTreeNodeBase* pChild, int ind )  {
	if (pChild)   {
		if (ind >= 0)  {
			_child[ind] = pChild;
		}
		pChild->_childIndex = ind;
		int simbling = 0;
		for (auto p = pChild; p != nullptr; p = p->getNextNode())  {    //�����ֵܽڵ㶼Ҫ�и��ڵ�
			p->_pParent = this;
			simbling++;
		}
		pChild->_siblings = ((simbling == 0) ? 1 : simbling);     //������ͬ���ֵܽڵ�ĸ���,�������и��ڵ㺢�ӵĸ���
	}
}

SyntaxTreeNodeBase* AssignStatement::getChildByTag(string name)
{
	if (name == "var_name")  {
		return _child[AssignLetf];
	}
	else if (name == "var_rval")  {
		return _child[AssignRight];
	}
	return nullptr;
}

SyntaxTreeNodeBase* AssignStatement::getAssginLeft()
{
	return _child[AssignLetf];
}

SyntaxTreeNodeBase* AssignStatement::getAssginRight()
{
	return _child[AssignRight];
}




SyntaxTreeNodeBase* VarDecNode::getVarDecType()
{
	return _child[VarDecNode::VarDec_Type];
}

SyntaxTreeNodeBase* VarDecNode::getVarDecName()
{
	return _child[VarDecNode::VarDec_Name];
}











void TreeNodeList::Push(TreeNode* node)  
{     
	if (node != nullptr)  {
		if (_head == nullptr)  {
			TreeNode* curNode = getCurNode(node);
			if (curNode != node)  {  //Ҫ����Ľڵ��Ǹ����ڵ���Ҫ��ɢһ��һ���ļ�
				_head = node;
				_cur = curNode;
			}
			else  {
				_head = _cur = node;
			}
		}
		else  {
			TreeNode* curNode = getCurNode(node);  //�ڵ�ĵ�ǰ�ڵ�,�����һ���ڵ�
			if (curNode != node)  {                //Ҫ����Ľڵ��Ǹ����ڵ���Ҫ��ɢһ��һ���ļ�
				_cur->setNextNode(node);
				_cur = curNode;
			}
			else  {
				_cur->setNextNode(node);
				_cur = node;
			}
		}
	}
}

TreeNode* TreeNodeList::getCurNode(TreeNode* node)  
{
	TreeNode* curNode = nullptr;
	while (node)  {
		curNode = node;
		node = node->getNextNode();
	}
	return curNode;
}

TreeNode* TreeNodeList::joinBy(TreeNodeList* node2)
{
	if (_cur)  {
		if (node2)  {
			_cur->setNextNode(node2->getHeadNode());
		}
	}
	else {
		if (node2)  {
			return node2->getHeadNode();
		}
	}
	return _head;
}