#ifndef UNICODE
#define UNICODE
#endif
#include "resource.h"
#include <array>
#include <ctime>
#include <random>
#include <string>
#include <vector>
#include <windows.h>
#include <KamataEngine.h>

using namespace KamataEngine;
// ウィンドウサイズ
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
const wchar_t kWindowClassName[] = L"NightWatchWindowClass";
const wchar_t kWindowTitle[] = L"夜の番人";

// ---- 時相 / ゲーム状態 ----
enum class TimePhase { Day, Night, Result };
enum class GameState { Title, Explanation, Play, GameOver };

// ---- ヘルパー: RGB によるブラシ作成 ----
static HBRUSH CreateSolidBrushRGB(int r, int g, int b) { return CreateSolidBrush(RGB(r, g, b)); }

// ---- Game クラス ----
class Game {
	public:
	Game() {
		std::random_device rd;
		rng_.seed(static_cast<unsigned long>(rd()));
		Reset();
		titleBitmap_ = nullptr;
		titleFont_ = nullptr;
		defaultFont_ = nullptr;
	}

	~Game() {
		if (titleBitmap_)
			DeleteObject(titleBitmap_);
		if (titleFont_)
			DeleteObject(titleFont_);
		if (defaultFont_)
			DeleteObject(defaultFont_);
	}
	
	void LoadResources(HINSTANCE hInstance, HWND hwnd) {
		(void)hwnd;

		titleBitmap_ = (HBITMAP)LoadImageW(hInstance, MAKEINTRESOURCEW(IDB_TITLE), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

		titleFont_ = CreateFontW(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Meiryo UI");

		defaultFont_ = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Meiryo UI");
	}

	void Reset() {
		state_ = GameState::Title;
		phase_ = TimePhase::Day;
		phaseTimer_ = 600;
		nightTimer_ = 300;
		dayCount_ = 1;

		baseHP_ = 100;
		playerAttack_ = 10;
		baseDefense_ = 5;

		points_ = 5;
		lastEarnedPoints_ = 0;

		dayPointsBackup_ = points_;
		attackBackup_ = playerAttack_;
		defenseBackup_ = baseDefense_;
		baseHPBackup_ = baseHP_;
		baseHPBackupNight_ = baseHP_;

		eventMessage_.clear();

		balls_.clear();
	}

	void Update(const std::array<BYTE, 256>& keys, const std::array<BYTE, 256>& preKeys) {
		switch (state_) {
		case GameState::Title:
			if (IsKeyTriggered(keys, preKeys, VK_RETURN)) {
				state_ = GameState::Explanation;
			}
			break;

		case GameState::Explanation:
			if (IsKeyTriggered(keys, preKeys, VK_RETURN)) {
				state_ = GameState::Play;
				phase_ = TimePhase::Day;
				phaseTimer_ = 600;
				dayCount_ = 1;
				baseHP_ = 100;
				points_ = 5;
				eventMessage_.clear();
				SpawnBalls(); // 初期スポーン
			}
			break;

		case GameState::Play:
			UpdatePlay(keys, preKeys);
			if (baseHP_ <= 0) {
				state_ = GameState::GameOver;
			}
			break;

		case GameState::GameOver:
			if (IsKeyTriggered(keys, preKeys, VK_RETURN)) {
				Reset();
			}
			break;
		}
	}

	void Draw(HDC backDC, int backW, int backH) {
		
		HFONT oldFont = nullptr;
		if (defaultFont_)
			oldFont = (HFONT)SelectObject(backDC, defaultFont_);

		switch (state_) {
		case GameState::Title:
			DrawTitle(backDC, backW, backH);
			break;
		case GameState::Explanation:
			DrawExplanation(backDC);
			break;
		case GameState::Play:
			DrawPlay(backDC);
			break;
		case GameState::GameOver:
			DrawGameOver(backDC);
			break;
		}

		if (oldFont)
			SelectObject(backDC, oldFont);
	}

private:
	// ---------- ボール関連 ----------
	struct Ball {
		float x;
		float y;
		float speed; // px / フレーム
		int type;    // 0 = red, 1 = blue, 2 = gray, 3 = yellow
	};

	std::vector<Ball> balls_;

	// 夜のフレーム数（コード内の設定に合わせる）
	static constexpr int kNightFramesDefault = 300;

	// スピード比（グレーを基準にして各色を比率で決める）
	// グレーが最速で、夜終了時にだいたい画面3/4に到達するように計算する
	float ComputeGraySpeed() const {
		// 期待する移動先 = 画面幅の3/4
		// 出現位置は左半分ランダム（平均は 1/4 幅付近）と想定 → 必要距離おおよそ 0.5 * width
		// distance = 0.75W - 0.25W = 0.5W
		float distance = (kWindowWidth * 0.5f);
		float frames = static_cast<float>(kNightFramesDefault);
		return distance / frames;
	}

	void SpawnBalls() {
		balls_.clear();

		int redCount = 3;
		int blueCount = (dayCount_ >= 6) ? 2 : 0;
		int grayCount = (dayCount_ >= 11) ? 2 : 0;
		int yellowCount = (dayCount_ >= 16) ? 1 : 0;

		// グレー基準速度を計算
		float graySpeed = ComputeGraySpeed();

		// 各色速度（比率）
		float redSpeed = graySpeed * 0.40f;    // 一番遅い
		float blueSpeed = graySpeed * 0.70f;   // 次
		float yellowSpeed = graySpeed * 0.90f; // 次
		// graySpeed は最速

		// RNG: 左半分からランダムにスタートする（0 .. kWindowWidth/2 - 1）
		std::uniform_int_distribution<int> distX(0, kWindowWidth / 2 - 1);
		std::uniform_int_distribution<int> distY(0, kWindowHeight - 64);

		for (int i = 0; i < redCount; ++i) {
			balls_.push_back(Ball{static_cast<float>(distX(rng_)), static_cast<float>(distY(rng_)), redSpeed, 0});
		}
		for (int i = 0; i < blueCount; ++i) {
			balls_.push_back(Ball{static_cast<float>(distX(rng_)), static_cast<float>(distY(rng_)), blueSpeed, 1});
		}
		for (int i = 0; i < grayCount; ++i) {
			balls_.push_back(Ball{static_cast<float>(distX(rng_)), static_cast<float>(distY(rng_)), graySpeed, 2});
		}
		for (int i = 0; i < yellowCount; ++i) {
			balls_.push_back(Ball{static_cast<float>(distX(rng_)), static_cast<float>(distY(rng_)), yellowSpeed, 3});
		}
	}

	void UpdateBallsDuringNight() {
		for (auto& b : balls_) {
			b.x += b.speed;
			// 右端を超えたら簡易的に画面外へ送り出す（念のため）
			if (b.x > kWindowWidth + 64.0f) {
				b.x = -64.0f;
			}
		}
	}

	void DrawBalls(HDC hdc) {
		// 64x64 の丸を各色で描画（ブラシを都度作って削除）
		for (const auto& b : balls_) {
			HBRUSH brush = nullptr;
			switch (b.type) {
			case 0:
				brush = CreateSolidBrushRGB(220, 30, 30);
				break; // 赤
			case 1:
				brush = CreateSolidBrushRGB(30, 120, 220);
				break; // 青
			case 2:
				brush = CreateSolidBrushRGB(170, 170, 170);
				break; // グレー
			case 3:
				brush = CreateSolidBrushRGB(240, 200, 40);
				break; // 黄
			default:
				brush = CreateSolidBrushRGB(255, 255, 255);
				break;
			}
			HBRUSH old = (HBRUSH)SelectObject(hdc, brush);
			// Ellipse の矩形を 64x64 に調整
			Ellipse(hdc, (int)b.x, (int)b.y, (int)(b.x + 64.0f), (int)(b.y + 64.0f));
			SelectObject(hdc, old);
			DeleteObject(brush);
		}
	}

	// ---------- ゲーム本体 ----------
	void StartDay() {
		phase_ = TimePhase::Day;
		phaseTimer_ = 600;
		dayCount_++;
		points_ += lastEarnedPoints_;
		eventMessage_.clear();

		SpawnBalls();
	}

	void StartNight() {
		phase_ = TimePhase::Night;
		nightTimer_ = 300;
		baseHPBackupNight_ = baseHP_;
		// 夜が始まったら（移動は夜中だけ）特に初期化は不要
	}

	void StartResult() {
		phase_ = TimePhase::Result;

		int enemyCount = dayCount_ * 5;

		double defenseRate = 0.5;
		if (dayCount_ <= 5)
			defenseRate = 1.0;
		else if (dayCount_ <= 10)
			defenseRate = 0.8;
		else if (dayCount_ <= 15)
			defenseRate = 0.666;

		int effectiveDefense = static_cast<int>((playerAttack_ + baseDefense_) * defenseRate);
		int damage = enemyCount * 3 - effectiveDefense;

		if (damage > 0)
			baseHP_ -= damage;
		if (baseHP_ < 0)
			baseHP_ = 0;

		lastEarnedPoints_ = enemyCount / 2;

		HandleEvent();
	}

	void UpdatePlay(const std::array<BYTE, 256>& keys, const std::array<BYTE, 256>& preKeys) {
		int attackCost = playerAttack_ / 100 + 1;
		int defenseCost = baseDefense_ / 100 + 1;

		switch (phase_) {
		case TimePhase::Day:
			if (phaseTimer_ == 600) {
				dayPointsBackup_ = points_;
				attackBackup_ = playerAttack_;
				defenseBackup_ = baseDefense_;
				baseHPBackup_ = baseHP_;
			}

			phaseTimer_--;

			if (IsKeyTriggered(keys, preKeys, '1') && points_ >= attackCost) {
				playerAttack_ += 2;
				points_ -= attackCost;
			}
			if (IsKeyTriggered(keys, preKeys, '2') && points_ >= defenseCost) {
				baseDefense_ += 2;
				points_ -= defenseCost;
			}
			if (IsKeyTriggered(keys, preKeys, '3') && points_ > 0) {
				baseHP_ += 10;
				if (baseHP_ > 200)
					baseHP_ = 200;
				points_ -= 1;
			}

			if (IsKeyTriggered(keys, preKeys, 'R')) {
				points_ = dayPointsBackup_;
				playerAttack_ = attackBackup_;
				baseDefense_ = defenseBackup_;
				baseHP_ = baseHPBackup_;
			}

			if (IsKeyTriggered(keys, preKeys, VK_RETURN) || phaseTimer_ <= 0) {
				StartNight();
			}
			break;

		case TimePhase::Night:
			nightTimer_--;
			// 夜中はボールを動かす
			UpdateBallsDuringNight();
			if (nightTimer_ <= 0)
				StartResult();
			break;

		case TimePhase::Result:
			if (IsKeyTriggered(keys, preKeys, VK_RETURN)) {
				StartDay();
			}
			break;
		}
	}

	void HandleEvent() {
		eventMessage_.clear();

		std::uniform_int_distribution<int> dist10000(0, 9999);
		if (dist10000(rng_) < 2) {
			playerAttack_ += dayCount_ * 10;
			baseDefense_ += dayCount_ * 10;
			lastEarnedPoints_ *= 3;
			eventMessage_ = L"【覚醒】力がみなぎる！（ポイント3倍）";
			return;
		}

		std::uniform_int_distribution<int> dist100(0, 99);
		int roll = dist100(rng_);

		if (roll < 30) {
			int goodType = std::uniform_int_distribution<int>(0, 3)(rng_);
			switch (goodType) {
			case 0:
				playerAttack_ += dayCount_ * 5;
				eventMessage_ = L"いい武器を拾った！";
				break;
			case 1:
				baseDefense_ += dayCount_ * 5;
				eventMessage_ = L"いい防具を拾った！";
				break;
			case 2:
				lastEarnedPoints_ *= 2;
				eventMessage_ = L"今日は調子が良い！（ポイント2倍）";
				break;
			case 3:
				baseHP_ = 200;
				eventMessage_ = L"シスターに癒された！（HP全回復）";
				break;
			}
		} else if (roll < 50) {
			int badType = std::uniform_int_distribution<int>(0, 2)(rng_);
			switch (badType) {
			case 0:
				playerAttack_ -= dayCount_;
				if (playerAttack_ < 0)
					playerAttack_ = 0;
				eventMessage_ = L"武器が壊れた…";
				break;
			case 1:
				baseDefense_ -= dayCount_;
				if (baseDefense_ < 0)
					baseDefense_ = 0;
				eventMessage_ = L"防具が壊れた…";
				break;
			case 2:
				lastEarnedPoints_ /= 2;
				eventMessage_ = L"今日は調子が悪い…（ポイント半減）";
				break;
			}
		} else {
			eventMessage_ = L"今日は特に何も起きなかった…。";
		}
	}

	void DrawTitle(HDC hdc, int w, int h) {
		if (titleBitmap_) {
			HDC mem = CreateCompatibleDC(hdc);
			HBITMAP old = (HBITMAP)SelectObject(mem, titleBitmap_);
			BITMAP bm;
			GetObject(titleBitmap_, sizeof(BITMAP), &bm);
			StretchBlt(hdc, 0, 0, w, h, mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
			SelectObject(mem, old);
			DeleteDC(mem);
		} else {
			RECT r = {0, 0, w, h};
			HBRUSH brush = CreateSolidBrushRGB(0, 0, 0);
			FillRect(hdc, &r, brush);
			DeleteObject(brush);
		}

		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkMode(hdc, TRANSPARENT);

		if (titleFont_) {
			HFONT old = (HFONT)SelectObject(hdc, titleFont_);
			TextOutW(hdc, 40, 40, kWindowTitle, static_cast<int>(wcslen(kWindowTitle)));
			SelectObject(hdc, old);
		} else {
			TextOutW(hdc, 40, 40, kWindowTitle, static_cast<int>(wcslen(kWindowTitle)));
		}

		TextOutW(hdc, 480, 600, L"Press ENTER", 12);
	}

	void DrawExplanation(HDC hdc) {
		RECT r = {0, 0, kWindowWidth, kWindowHeight};
		HBRUSH brush = CreateSolidBrushRGB(32, 32, 32);
		FillRect(hdc, &r, brush);
		DeleteObject(brush);

		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkMode(hdc, TRANSPARENT);

		TextOutW(hdc, 200, 200, L"【遊び方】", 6);
		TextOutW(hdc, 200, 240, L"昼：ポイントを使って強化", 12);
		TextOutW(hdc, 200, 260, L"   [1] Attack UP (+2)", 19);
		TextOutW(hdc, 200, 280, L"   [2] Defense UP (+2)", 20);
		TextOutW(hdc, 200, 300, L"   [3] Heal Base (+10)", 20);
		TextOutW(hdc, 200, 320, L"   [R] Undo All Actions This Turn", 33);
		TextOutW(hdc, 200, 340, L"夜：敵が自動で攻撃、朝に結果報告", 20);
		TextOutW(hdc, 480, 500, L"Press ENTER to Play", 20);
	}

	void DrawPlay(HDC hdc) {
		RECT r = {0, 0, kWindowWidth, kWindowHeight};
		HBRUSH brush = nullptr;

		if (phase_ == TimePhase::Day)
			brush = CreateSolidBrushRGB(135, 206, 235); // 青空っぽい
		else if (phase_ == TimePhase::Night)
			brush = CreateSolidBrushRGB(10, 10, 42); // 夜
		else
			brush = CreateSolidBrushRGB(255, 204, 153); // 朝っぽい

		FillRect(hdc, &r, brush);
		DeleteObject(brush);

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(0, 0, 0));

		std::wstring buf;

		buf = L"Day: " + std::to_wstring(dayCount_);
		TextOutW(hdc, 20, 20, buf.c_str(), (int)buf.size());

		buf = L"HP: " + std::to_wstring(baseHP_) + L" / 200";
		TextOutW(hdc, 20, 40, buf.c_str(), (int)buf.size());

		buf = L"Attack: " + std::to_wstring(playerAttack_);
		TextOutW(hdc, 20, 60, buf.c_str(), (int)buf.size());

		buf = L"Defense: " + std::to_wstring(baseDefense_);
		TextOutW(hdc, 20, 80, buf.c_str(), (int)buf.size());

		buf = L"Points: " + std::to_wstring(points_);
		TextOutW(hdc, 20, 100, buf.c_str(), (int)buf.size());

		// ================================
		// ★ 白丸をフェーズごとに配置する処理を変更
		//  昼: 画面中央
		//  夜: 画面の右1/4中央（x = 3/4 * width の位置に 64x64 を中央揃えで描画）
		//  朝(Result): 昼と同じ（中央）
		// ================================
		{
			int cx, cy;
			int radius = 32; // 半径 32 => 直径 64 (64x64)
			cy = kWindowHeight / 2;

			// ◆ 白丸の描画（昼・夜のみ表示 → Result では描かない）
			if (phase_ == TimePhase::Night) { 
				// 右1/4の中央に配置する -> x = 3/4 * width の中心位置 
				cx = (kWindowWidth * 3) / 4; 
			} else 
			{   // 昼・朝は中央 
				cx = kWindowWidth / 2;
			}
			// Ellipse expects left, top, right, bottom
			HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
			HBRUSH old = (HBRUSH)SelectObject(hdc, whiteBrush);
			Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
			SelectObject(hdc, old);
			DeleteObject(whiteBrush);
		}

		// ===== 以下いつものUI描画 =====

		if (phase_ == TimePhase::Day) {
			int attackCost = playerAttack_ / 100 + 1;
			int defenseCost = baseDefense_ / 100 + 1;

			TextOutW(hdc, 20, 140, L"=== Day Phase ===", 18);

			buf = L"[1] Attack UP (+2)  Cost: " + std::to_wstring(attackCost);
			TextOutW(hdc, 20, 160, buf.c_str(), (int)buf.size());

			buf = L"[2] Defense UP (+2) Cost: " + std::to_wstring(defenseCost);
			TextOutW(hdc, 20, 180, buf.c_str(), (int)buf.size());

			TextOutW(hdc, 20, 200, L"[3] Heal Base (+10) Cost: 1", 29);
			TextOutW(hdc, 20, 220, L"[R] Undo All Actions This Turn", 33);
			TextOutW(hdc, 20, 240, L"[ENTER] Skip to Night", 21);

		} else if (phase_ == TimePhase::Night) {
			TextOutW(hdc, 20, 140, L"=== Night Phase ===", 19);
			TextOutW(hdc, 20, 160, L"Enemies attack... Survive until morning!", 41);

			// 夜は色付きボール（敵）を描画（位置は夜中に毎フレーム更新される）
			DrawBalls(hdc);

		} else if (phase_ == TimePhase::Result) {
			TextOutW(hdc, 20, 140, L"=== Morning Report ===", 23);

			buf = L"Last Earned Points: " + std::to_wstring(lastEarnedPoints_);
			TextOutW(hdc, 20, 160, buf.c_str(), (int)buf.size());

			int lostHP = baseHPBackupNight_ - baseHP_;
			if (lostHP < 0)
				lostHP = 0;

			buf = L"HP Lost Last Night: " + std::to_wstring(lostHP);
			TextOutW(hdc, 20, 180, buf.c_str(), (int)buf.size());

			if (!eventMessage_.empty())
				TextOutW(hdc, 20, 200, eventMessage_.c_str(), (int)eventMessage_.size());

			TextOutW(hdc, 20, 240, L"Press ENTER to Continue", 24);
		}
	}

	void DrawGameOver(HDC hdc) {
		RECT r = {0, 0, kWindowWidth, kWindowHeight};
		HBRUSH brush = CreateSolidBrushRGB(0, 0, 0);
		FillRect(hdc, &r, brush);
		DeleteObject(brush);

		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkMode(hdc, TRANSPARENT);

		TextOutW(hdc, 500, 300, L"GAME OVER", 9);
		std::wstring buf = L"Survived " + std::to_wstring(dayCount_) + L" Days";
		TextOutW(hdc, 480, 360, buf.c_str(), (int)buf.size());
		TextOutW(hdc, 460, 420, L"Press ENTER to Title", 21);
	}

	static bool IsKeyTriggered(const std::array<BYTE, 256>& keys, const std::array<BYTE, 256>& preKeys, int vk) { return keys[vk] && !preKeys[vk]; }

private:
	GameState state_;
	TimePhase phase_;
	int phaseTimer_;
	int nightTimer_;
	int dayCount_;

	int baseHP_;
	int playerAttack_;
	int baseDefense_;

	int points_;
	int lastEarnedPoints_;

	int dayPointsBackup_;
	int attackBackup_;
	int defenseBackup_;
	int baseHPBackup_;
	int baseHPBackupNight_;

	std::wstring eventMessage_;

	HBITMAP titleBitmap_;
	HFONT titleFont_;
	HFONT defaultFont_;

	std::mt19937 rng_;
};

// ウィンドウプロシージャ等は変更なし
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	//uint32_t noon = TextureManager::Load("noon.png");
	//Sprite* sprite = Sprite::Create(noon, {0.0f, 0.0f});
	//sprite->SetTextureRect({0, 0}, {1280, 720});
	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = kWindowClassName;
	RegisterClassExW(&wc);

	RECT wr = {0, 0, kWindowWidth, kWindowHeight};
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hwnd = CreateWindowW(kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	if (!hwnd)
		return -1;

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	HDC frontDC = GetDC(hwnd);
	HDC backDC = CreateCompatibleDC(frontDC);
	HBITMAP backBitmap = CreateCompatibleBitmap(frontDC, kWindowWidth, kWindowHeight);
	HBITMAP oldBackBmp = (HBITMAP)SelectObject(backDC, backBitmap);

	Game game;
	game.LoadResources(hInstance, hwnd);

	std::array<BYTE, 256> keys = {};
	std::array<BYTE, 256> preKeys = {};

	bool running = true;
	MSG msg = {};

	const int targetFPS = 60;
	const int frameMs = 1000 / targetFPS;

	while (running) {
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				running = false;
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		preKeys = keys;
		for (int i = 0; i < 256; ++i)
			keys[i] = (GetAsyncKeyState(i) & 0x8000) ? 1 : 0;

		if (keys[VK_ESCAPE] && !preKeys[VK_ESCAPE])
			running = false;

		game.Update(keys, preKeys);

		game.Draw(backDC, kWindowWidth, kWindowHeight);

		BitBlt(frontDC, 0, 0, kWindowWidth, kWindowHeight, backDC, 0, 0, SRCCOPY);

		Sleep(frameMs);
		// 描画開始
		// スプライト描画前処理

		//Sprite::PreDraw();

		// スプライトを描画

		//sprite->Draw();

		// スプライト描画後処理

		//Sprite::PostDraw();
	}
	//delete sprite;
	SelectObject(backDC, oldBackBmp);
	DeleteObject(backBitmap);
	DeleteDC(backDC);
	ReleaseDC(hwnd, frontDC);

	DestroyWindow(hwnd);
	return 0;
}
