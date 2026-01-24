#pragma once

// コマンド基底クラス
class CommandBase
{
public:
	virtual ~CommandBase() {}

	// 実行 (Redoも兼ねる)
	virtual void Execute() = 0;

	// 元に戻す
	virtual void Undo() = 0;
};
