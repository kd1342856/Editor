#pragma once

//============================================================
// アプリケーションクラス
//	APP.～ でどこからでもアクセス可能
//============================================================
class Application
{
// メンバ
public:

private:

	void KdPostDraw();
	void KdBeginDraw(bool usePostProcess);
	void KdPostUpdate();
	void KdBeginUpdate();

	// ゲーム終了フラグ trueで終了する
	bool		m_endFlag = false;

//=====================================================
// シングルトンパターン
//=====================================================
private:
	// 
	Application() {}

public:
	static Application &Instance(){
		static Application Instance;
		return Instance;
	}
};
