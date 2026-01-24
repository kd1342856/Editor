#pragma once
#include "CommandBase.h"

// Transform変更コマンド
class CmdTransform : public CommandBase
{
public:
	CmdTransform(const std::weak_ptr<Entity>& target, const Math::Matrix& prevMat, const Math::Matrix& nextMat)
		: m_target(target), m_prevMat(prevMat), m_nextMat(nextMat)
	{
	}

	void Execute() override
	{
		ApplyMatrix(m_nextMat);
	}

	void Undo() override
	{
		ApplyMatrix(m_prevMat);
	}

private:
	void ApplyMatrix(const Math::Matrix& mat)
	{
		auto ent = m_target.lock();
		if (!ent) return;

		auto tc = ent->GetComponent<TransformComponent>();
		if (!tc) return;

		// ImGuizmoの分解関数を使用 (Euler Deg が返ってくるのでそのままセットできる)
		float t[3], r[3], s[3];
		ImGuizmo::DecomposeMatrixToComponents(&mat._11, t, r, s);

		tc->SetPosition({ t[0], t[1], t[2] });
		tc->SetRotation({ r[0], r[1], r[2] });
		tc->SetScale({ s[0], s[1], s[2] });
	}

	std::weak_ptr<Entity> m_target;
	Math::Matrix m_prevMat;
	Math::Matrix m_nextMat;
};
