#pragma once
#include "CommandBase.h"
class CommandManager
{
public:
	// コマンドを実行して履歴に追加
	void Execute(const std::shared_ptr<CommandBase>& cmd)
	{
		if (!cmd) return;

		// 実行
		cmd->Execute();

		// Undoスタックに積む
		m_undoStack.push(cmd);

		// 新しい動作をしたのでRedoスタックはクリア
		while (!m_redoStack.empty())
		{
			m_redoStack.pop();
		}
	}

	// Undo
	void Undo()
	{
		if (m_undoStack.empty()) return;

		auto cmd = m_undoStack.top();
		m_undoStack.pop();

		cmd->Undo();

		m_redoStack.push(cmd);
	}

	// Redo
	void Redo()
	{
		if (m_redoStack.empty()) return;

		auto cmd = m_redoStack.top();
		m_redoStack.pop();

		cmd->Execute(); // Execute = Redo

		m_undoStack.push(cmd);
	}

	// 履歴クリア
	void Clear()
	{
		while (!m_undoStack.empty()) m_undoStack.pop();
		while (!m_redoStack.empty()) m_redoStack.pop();
	}

private:
	CommandManager() {}
	~CommandManager() {}

	std::stack<std::shared_ptr<CommandBase>> m_undoStack;
	std::stack<std::shared_ptr<CommandBase>> m_redoStack;

public:
	// シングルトンパターン 
	static CommandManager& Instance()
	{
		static CommandManager instance;
		return instance;
	}
};
