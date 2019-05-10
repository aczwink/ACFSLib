/*
 * Copyright (c) 2019 Amir Czwink (amir130@hotmail.de)
 *
 * This file is part of ACFSLib.
 *
 * ACFSLib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ACFSLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ACFSLib.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <Std++.hpp>
using namespace StdXX;
using namespace StdXX::UI;

struct FSNode
{
	Path path;
	mutable bool childrenRead;
	mutable DynamicArray<UniquePointer<FSNode>> children;
	FSNode* parent;

	FSNode(const Path& path) : path(path), childrenRead(false), parent(nullptr)
	{
	}
};

class ExampleController : public TreeController
{
public:
	ExampleController()
	{
		this->root = new FSNode(String(u8"/home/amir/git-repositories/"));
	}

	//Methods
	ControllerIndex GetChildIndex(uint32 row, uint32 column, const ControllerIndex & parent = ControllerIndex()) const override
	{
		const FSNode* node;
		if (parent.HasParent())
			node = (const FSNode*)parent.GetNode();
		else
			node = this->root.operator->();
		this->MakeSureChildrenRead(*node);

		if (row != Unsigned<uint32>::Max())
			node = node->children[row].operator->();

		return this->CreateIndex(row, column, node);
	}

	String GetColumnText(uint32 column) const override
	{
		const String cols[] = { u8"Name", u8"Size", u8"Type" };
		return cols[column];
	}

	uint32 GetNumberOfChildren(const ControllerIndex & parent = ControllerIndex()) const override
	{
		const FSNode* node;
		if (parent.HasParent())
			node = (const FSNode*)parent.GetNode();
		else
			node = this->root.operator->();
		this->MakeSureChildrenRead(*node);
		return node->children.GetNumberOfElements();
	}

	uint32 GetNumberOfColumns() const override
	{
		return 3;
	}

	ControllerIndex GetParentIndex(const ControllerIndex & index) const override
	{
		FSNode* node = (FSNode*)index.GetNode();
		if (node == this->root.operator->())
			return {};
		uint32 row = 0;
		for (const auto& childNode : node->parent->children)
		{
			if (childNode.operator->() == node)
				break;
			row++;
		}
		return this->CreateIndex(row, Unsigned<uint32>::Max(), node->parent);
	}

	String GetText(const ControllerIndex & index) const override
	{
		const FSNode* node = (const FSNode*)index.GetNode();
		switch (index.GetColumn())
		{
			case 0:
				return node->path.GetTitle();
		}
		const String data[] = { u8"example.png", u8"100 bytes", u8"png" };
		return data[index.GetColumn()];
	}

private:
	//Members
	UniquePointer<FSNode> root;

	//Methods
	void MakeSureChildrenRead(const FSNode& node) const
	{
		if (node.childrenRead)
			return;

		node.childrenRead = true;
		if (OSFileSystem::GetInstance().IsDirectory(node.path))
		{
			AutoPointer<Directory> dir = OSFileSystem::GetInstance().GetDirectory(node.path);
			for (const auto& childName : *dir)
			{
				FSNode* childNode = new FSNode(node.path / childName);
				childNode->parent = (FSNode*)&node;
				node.children.Push(childNode);
			}
		}
	}
};

int32 Main(const String &programName, const FixedArray<String> &args)
{
	StandardEventQueue eventQueue;
	MainAppWindow* wnd = new MainAppWindow(eventQueue);

	TreeView* treeView = new TreeView;
	treeView->SetController(new ExampleController);
	wnd->AddContentChild(treeView);

	wnd->Show();

	eventQueue.ProcessEvents();

	return EXIT_SUCCESS;
}