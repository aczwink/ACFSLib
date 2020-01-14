/*
 * Copyright (c) 2019-2020 Amir Czwink (amir130@hotmail.de)
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

struct BufferedFileSystemNode
{
	Path path;
	mutable bool childrenRead;
	mutable DynamicArray<UniquePointer<BufferedFileSystemNode>> children;
	BufferedFileSystemNode* parent;
	mutable uint64 size;
	mutable FileSystemNodeType type;

	BufferedFileSystemNode(const Path& path) : path(path), childrenRead(false), parent(nullptr), size(0)
	{
	}
};

class ExampleController : public TreeController
{
public:
	ExampleController()
	{
		this->root = new BufferedFileSystemNode(String(u8"/home/amir/Schreibtisch/"));
	}

	//Methods
	ControllerIndex GetChildIndex(uint32 row, uint32 column, const ControllerIndex & parent = ControllerIndex()) const override
	{
		const BufferedFileSystemNode* node;
		if (parent.HasParent())
			node = (const BufferedFileSystemNode*)parent.GetNode();
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
		const BufferedFileSystemNode* node;
		if (parent.HasParent())
			node = (const BufferedFileSystemNode*)parent.GetNode();
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
		BufferedFileSystemNode* node = (BufferedFileSystemNode*)index.GetNode();
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
		const BufferedFileSystemNode* node = (const BufferedFileSystemNode*)index.GetNode();
		switch (index.GetColumn())
		{
			case 0:
				return node->path.GetName();
			case 1:
				this->MakeSureChildrenRead(*node);
				return String::FormatBinaryPrefixed(node->size);
			case 2:
			{
				switch(node->type)
				{
					case FileSystemNodeType::Directory:
						return u8"Directory";
						break;
					case FileSystemNodeType::File:
						return u8"File";
						break;
					case FileSystemNodeType::Link:
						return u8"Link";
				}
			}
		}
		return u8"";
	}

private:
	//Members
	UniquePointer<BufferedFileSystemNode> root;

	//Methods
	void MakeSureChildrenRead(const BufferedFileSystemNode& node) const
	{
		if (node.childrenRead)
			return;

		node.childrenRead = true;
		AutoPointer<const FileSystemNode> fsNode = OSFileSystem::GetInstance().GetNode(node.path);
		node.type = fsNode->GetType();

		if(fsNode->GetType() == FileSystemNodeType::File)
		{
			node.size = fsNode.Cast<const File>()->GetSize();
		}
		else
		{
			node.size = fsNode->QueryInfo().storedSize;
		}

		if (fsNode->GetType() == FileSystemNodeType::Directory)
		{
			AutoPointer<const Directory> dir = fsNode.Cast<const Directory>();
			for (const auto& childName : *dir)
			{
				BufferedFileSystemNode* childNode = new BufferedFileSystemNode(node.path / childName);
				childNode->parent = (BufferedFileSystemNode*)&node;
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