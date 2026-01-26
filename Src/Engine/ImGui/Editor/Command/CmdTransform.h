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

		auto cTrans = ent->GetComponent<TransformComponent>();
		if (!cTrans) return;

		// ImGuizmoの分解関数を使用
		float trans[3], rot[3], scale[3];
		ImGuizmo::DecomposeMatrixToComponents(&mat._11, trans, rot, scale);

		cTrans->SetPosition({	trans[0], trans[1]	, trans[2] });
		cTrans->SetRotation({	rot[0]	, rot[1]	, rot[2] });
		cTrans->SetScale({		scale[0], scale[1]	, scale[2] });
	}

	std::weak_ptr<Entity> m_target;
	Math::Matrix m_prevMat;
	Math::Matrix m_nextMat;
};
