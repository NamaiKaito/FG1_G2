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
//タイトル
uint32_t textureTitle_ = 0;
KamataEngine::Sprite* spriteTitle_ = nullptr;
uint32_t textureEnter_ = 0;
KamataEngine::Sprite* spriteEnter_ = nullptr;
//説明
uint32_t textureExplanation_ = 0;
KamataEngine::Sprite* spriteExplanation_ = nullptr;
uint32_t textureRule_ = 0;
KamataEngine::Sprite* spriteRule_ = nullptr;
//結果
uint32_t textureResult_ = 0;
KamataEngine::Sprite* spriteResult_ = nullptr;
//昼
uint32_t textureNoon_ = 0;
KamataEngine::Sprite* spriteNoon_ = nullptr;
//夜
uint32_t textureNight_ = 0;
KamataEngine::Sprite* spriteNight_ = nullptr;
//朝
uint32_t textureMorning_ = 0;
KamataEngine::Sprite* spriteMorning_ = nullptr;
//ゲームオーバー
uint32_t textureGameOver_ = 0;
KamataEngine::Sprite* spriteGameOver_ = nullptr;
//プレイヤー
uint32_t texturePlayer_ = 0;
KamataEngine::Sprite* spritePlayer_ = nullptr;
//横向きプレイヤー
uint32_t texturePlayer2_ = 0;
KamataEngine::Sprite* spritePlayer2_ = nullptr;
//ステータス
uint32_t textureStatus_ = 0;
KamataEngine::Sprite* spriteStatus_ = nullptr;
//ポイント
uint32_t texturePoint_ = 0;
KamataEngine::Sprite* spritePoint_ = nullptr;
//HP
uint32_t textureLife_ = 0;
KamataEngine::Sprite* spriteLife_ = nullptr;
//回復
uint32_t textureHeal_ = 0;
KamataEngine::Sprite* spriteHeal_ = nullptr;
//剣
uint32_t textureSword_ = 0;
KamataEngine::Sprite* spriteSword_ = nullptr;
//攻撃力UP
uint32_t textureAttack_ = 0;
KamataEngine::Sprite* spriteAttack_ = nullptr;
//鎧
uint32_t textureArmor_ = 0;
KamataEngine::Sprite* spriteArmor_ = nullptr;
//防御力UP
uint32_t textureDefense_ = 0;
KamataEngine::Sprite* spriteDefense_ = nullptr;
//盾
//uint32_t textureShield_ = 0;
//KamataEngine::Sprite* spriteShield_ = nullptr;
//ゾンビ
uint32_t textureZombie_ = 0;
KamataEngine::Sprite* spriteZombie_ = nullptr;
//スケルトン
uint32_t textureSkeleton_ = 0;
KamataEngine::Sprite* spriteSkeleton_ = nullptr;
//狼男
uint32_t textureWerewolf_ = 0;
KamataEngine::Sprite* spriteWerewolf_ = nullptr;
//死神
uint32_t textureReaper_ = 0;
KamataEngine::Sprite* spriteReaper_ = nullptr;
//数字
uint32_t textureNumber_ = 0;
KamataEngine::Sprite* spriteDayNumber_[2] = {};
KamataEngine::Sprite* spriteHPNumber_[3] = {};
KamataEngine::Sprite* spriteAttackNumber_[3] = {};
KamataEngine::Sprite* spriteDefenseNumber_[3] = {};
KamataEngine::Sprite* spritePointNumber_[3] = {};
KamataEngine::Sprite* spriteAttackCostNumber_[1] = {};
KamataEngine::Sprite* spriteDefenseCostNumber_[1] = {};
KamataEngine::Sprite* spriteHPCostNumber_[1] = {};
KamataEngine::Vector2 size = {32.0f, 64.0f};
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
	
	void LoadResources() {
		//シーン
		textureTitle_ = TextureManager::Load("title_bannin.png");
		spriteTitle_ = Sprite::Create(textureTitle_, {0, 0});

		textureEnter_ = TextureManager::Load("enter.png");
		spriteEnter_ = Sprite::Create(textureEnter_, {0, 0});

		textureExplanation_ = TextureManager::Load("explanation.png");
		spriteExplanation_ = Sprite::Create(textureExplanation_, {0, 0});

		textureResult_ = TextureManager::Load("result.png");
		spriteResult_ = Sprite::Create(textureResult_, {120, -120});

		textureRule_ = TextureManager::Load("rule.png");
		spriteRule_ = Sprite::Create(textureRule_, {0, 0});

		textureNoon_ = TextureManager::Load("noon.png");
		spriteNoon_ = Sprite::Create(textureNoon_, {0, 0});

		textureNight_ = TextureManager::Load("night.png");
		spriteNight_ = Sprite::Create(textureNight_, {0, 0});

		textureMorning_ = TextureManager::Load("morning.png");
		spriteMorning_ = Sprite::Create(textureMorning_, {0, 0});

		textureGameOver_ = TextureManager::Load("gameOver.png");
		spriteGameOver_ = Sprite::Create(textureGameOver_, {0, 0});
		//フォント
		textureStatus_ = TextureManager::Load("status.png");
		spriteStatus_ = Sprite::Create(textureStatus_, {20, 0});

		textureHeal_ = TextureManager::Load("heal.png");
		spriteHeal_ = Sprite::Create(textureHeal_, {10, 298});

		textureLife_ = TextureManager::Load("life.png");
		spriteLife_ = Sprite::Create(textureLife_, {10, 55});

		textureSword_ = TextureManager::Load("sword.png");
		spriteSword_ = Sprite::Create(textureSword_, {10, 95});

		textureAttack_ = TextureManager::Load("attack.png");
		spriteAttack_ = Sprite::Create(textureAttack_, {10, 220});

		textureArmor_ = TextureManager::Load("armor.png");
		spriteArmor_ = Sprite::Create(textureArmor_, {10, 135});

		textureDefense_ = TextureManager::Load("defense.png");
		spriteDefense_ = Sprite::Create(textureDefense_, {10, 255});

		///*textureShield_ = TextureManager::Load("shield.png");
		//spriteShield_ = Sprite::Create(textureShield_, {10, 192});*/

		texturePoint_ = TextureManager::Load("point.png");
		spritePoint_ = Sprite::Create(texturePoint_, {10,175});
		//プレイヤー
		texturePlayer_ = TextureManager::Load("player.png");
		spritePlayer_ = Sprite::Create(texturePlayer_, {2000, 50});

		texturePlayer2_ = TextureManager::Load("player2.png");
		spritePlayer2_ = Sprite::Create(texturePlayer2_, {1100, 300});
		//敵キャラ
		textureZombie_ = TextureManager::Load("zombie.png");
		spriteZombie_ = Sprite::Create(textureZombie_, {0, 0});

		textureSkeleton_ = TextureManager::Load("skeleton.png");
		spriteSkeleton_ = Sprite::Create(textureSkeleton_, {0, 64});

		textureWerewolf_ = TextureManager::Load("wolf.png");
		spriteWerewolf_ = Sprite::Create(textureWerewolf_, {0, 128});

		textureReaper_ = TextureManager::Load("reaper.png");
		spriteReaper_ = Sprite::Create(textureReaper_, {0, 192});
		//数字
		textureNumber_ = TextureManager::Load("number.png");

		for (int i = 0; i < 2; i++) {
			spriteDayNumber_[i] = Sprite::Create(textureNumber_, {100.0f + size.x * i, 3});
			spriteDayNumber_[i]->SetSize({32.0f, 64.0f});
		}
		for (int i = 0; i < 2; i++) {
			spriteDayNumber_[i]->SetTextureRect({0, 0}, {32, 64});
		}

		for (int i = 0; i < 3; i++) {
			spriteHPNumber_[i] = Sprite::Create(textureNumber_, {92.0f + size.x * i, 42});
			spriteHPNumber_[i]->SetSize({32.0f, 64.0f});
		}
		for (int i = 0; i < 3; i++) {
			spriteHPNumber_[i]->SetTextureRect({0, 0}, {32, 64});
		}

		for (int i = 0; i < 3; i++) {
			spriteAttackNumber_[i] = Sprite::Create(textureNumber_, {125.0f + size.x * i, 82});
			spriteAttackNumber_[i]->SetSize({32.0f, 64.0f});
		}
		for (int i = 0; i < 3; i++) {
			spriteAttackNumber_[i]->SetTextureRect({0, 0}, {32, 64});
		}

		for (int i = 0; i < 3; i++) {
			spriteDefenseNumber_[i] = Sprite::Create(textureNumber_, {144.0f + size.x * i, 122});
			spriteDefenseNumber_[i]->SetSize({32.0f, 64.0f});
		}
		for (int i = 0; i < 3; i++) {
			spriteDefenseNumber_[i]->SetTextureRect({0, 0}, {32, 64});
		}

		for (int i = 0; i < 3; i++) {
			spritePointNumber_[i] = Sprite::Create(textureNumber_, {125.0f + size.x * i, 162});
			spritePointNumber_[i]->SetSize({32.0f, 64.0f});
		}
		for (int i = 0; i < 3; i++) {
			spritePointNumber_[i]->SetTextureRect({0, 0}, {32, 64});
		}

		for (int i = 0; i < 1; i++) {
			spriteAttackCostNumber_[i] = Sprite::Create(textureNumber_, {302.0f + size.x * i, 203});
			spriteAttackCostNumber_[i]->SetSize({32.0f, 64.0f});
		}
		for (int i = 0; i < 1; i++) {
			spriteAttackCostNumber_[i]->SetTextureRect({0, 0}, {32, 64});
		}

		for (int i = 0; i < 1; i++) {
			spriteDefenseCostNumber_[i] = Sprite::Create(textureNumber_, {320.0f + size.x * i, 245});
			spriteDefenseCostNumber_[i]->SetSize({32.0f, 64.0f});
		}
		for (int i = 0; i < 1; i++) {
			spriteDefenseCostNumber_[i]->SetTextureRect({0, 0}, {32, 64});
		}

		for (int i = 0; i < 1; i++) {
			spriteHPCostNumber_[i] = Sprite::Create(textureNumber_, {260.0f + size.x * i, 285});
			spriteHPCostNumber_[i]->SetSize({32.0f, 64.0f});
		}
		for (int i = 0; i < 1; i++) {
			spriteHPCostNumber_[i]->SetTextureRect({0, 0}, {32, 64});
		}
	}
	void DrawDayNumber(int number) { 
		int32_t digit = 10;
		for (int i = 0; i < 2;i++) {
			int nowNumber = number/ digit;
			spriteDayNumber_[i]->SetTextureRect({size.x * nowNumber, 0}, {size.x,size.y});
			number %= digit;
			digit /= 10;
		}
		for (int i = 0; i < 2; i++) {
			spriteDayNumber_[i]->Draw();
		}
	}
	void DrawHPNumber(int number) {
		int32_t digit = 100;
		for (int i = 0; i < 3; i++) {
			int nowNumber = number / digit;
			spriteHPNumber_[i]->SetTextureRect({size.x * nowNumber, 0}, {size.x, size.y});
			number %= digit;
			digit /= 10;
		}
		for (int i = 0; i < 3; i++) {
			spriteHPNumber_[i]->Draw();
		}
	}
	void DrawAttackNumber(int number) {
		int32_t digit = 100;
		for (int i = 0; i < 3; i++) {
			int nowNumber = number / digit;
			spriteAttackNumber_[i]->SetTextureRect({size.x * nowNumber, 0}, {size.x, size.y});
			number %= digit;
			digit /= 10;
		}
		for (int i = 0; i < 3; i++) {
			spriteAttackNumber_[i]->Draw();
		}
	}
	void DrawDefenseNumber(int number) {
		int32_t digit = 100;
		for (int i = 0; i < 3; i++) {
			int nowNumber = number / digit;
			spriteDefenseNumber_[i]->SetTextureRect({size.x * nowNumber, 0}, {size.x, size.y});
			number %= digit;
			digit /= 10;
		}
		for (int i = 0; i < 3; i++) {
			spriteDefenseNumber_[i]->Draw();
		}
	}
	void DrawPointNumber(int number) {
		int32_t digit = 100;
		for (int i = 0; i < 3; i++) {
			int nowNumber = number / digit;
			spritePointNumber_[i]->SetTextureRect({size.x * nowNumber, 0}, {size.x, size.y});
			number %= digit;
			digit /= 10;
		}
		for (int i = 0; i < 3; i++) {
			spritePointNumber_[i]->Draw();
		}
	}
	void DrawAttackCostNumber(int number) {
		int32_t digit = 1;
		for (int i = 0; i < 1; i++) {
			int nowNumber = number / digit;
			spriteAttackCostNumber_[i]->SetTextureRect({size.x * nowNumber, 0}, {size.x, size.y});
			number %= digit;
			digit /= 10;
		}
		for (int i = 0; i < 1; i++) {
			spriteAttackCostNumber_[i]->Draw();
		}
	}
	void DrawDefenseCostNumber(int number) {
		int32_t digit = 1;
		for (int i = 0; i < 1; i++) {
			int nowNumber = number / digit;
			spriteDefenseCostNumber_[i]->SetTextureRect({size.x * nowNumber, 0}, {size.x, size.y});
			number %= digit;
			digit /= 10;
		}
		for (int i = 0; i < 1; i++) {
			spriteDefenseCostNumber_[i]->Draw();
		}
	}
	void DrawHPCostNumber(int number) {
		int32_t digit = 1;
		for (int i = 0; i < 1; i++) {
			int nowNumber = number / digit;
			spriteHPCostNumber_[i]->SetTextureRect({size.x * nowNumber, 0}, {size.x, size.y});
			number %= digit;
			digit /= 10;
		}
		for (int i = 0; i < 1; i++) {
			spriteHPCostNumber_[i]->Draw();
		}
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

		enemies_.clear();
	}

	void Update() {
		switch (state_) {
		case GameState::Title:
			if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
				state_ = GameState::Explanation;
			}
			break;

		case GameState::Explanation:
			if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
				state_ = GameState::Play;
				phase_ = TimePhase::Day;
				phaseTimer_ = 600;
				dayCount_ = 1;
				baseHP_ = 100;
				points_ = 5;
				eventMessage_.clear();
				SpawnEnemies(); // 初期スポーン
			}
			break;

		case GameState::Play:
			UpdatePlay();
			if (baseHP_ <= 0) {
				state_ = GameState::GameOver;
			}
			break;

		case GameState::GameOver:
			if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
				Reset();
			}
			break;
		}
	}

	void Draw() {
		
		/*HFONT oldFont = nullptr;
		if (defaultFont_)
			oldFont = (HFONT)SelectObject(backDC, defaultFont_);*/

		switch (state_) {
		case GameState::Title:
			spriteTitle_->Draw();
			spriteEnter_->Draw();
			break;
		case GameState::Explanation:
			spriteExplanation_->Draw();
			spriteRule_->Draw();
			break;
		case GameState::Play:
			DrawPlay();
			break;
		case GameState::GameOver:
			spriteGameOver_->Draw();
			break;
		}

		/*if (oldFont)
			SelectObject(backDC, oldFont);*/
	}

private:
	// ---------- 敵関連 ----------
	struct Enemy {
		float x;
		float y;
		float speed; // px / フレーム
		int type;    // 0 = ゾンビ, 1 = スケルトン, 2 = 狼男, 3 = 死神
		KamataEngine::Sprite* sprite = nullptr;

	};

	std::vector<Enemy> enemies_;

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

	void SpawnEnemies() {
		enemies_.clear();

		int zombieCount = 3;
		int skeletonCount = (dayCount_ >= 6) ? 2 : 0;
		int wolfCount = (dayCount_ >= 11) ? 2 : 0;
		int reaperCount = (dayCount_ >= 16) ? 1 : 0;

		// グレー基準速度を計算
		float graySpeed = ComputeGraySpeed();

		// 各色速度（比率）
		float redSpeed = graySpeed * 0.40f;    // 一番遅い
		float blueSpeed = graySpeed * 0.70f;   // 次
		float yellowSpeed = graySpeed * 0.90f; // 次
		// graySpeed は最速
		float y = 100.0f;
		// RNG: 左半分からランダムにスタートする（0 .. kWindowWidth/2 - 1）
		std::uniform_int_distribution<int> distX(0, kWindowWidth / 2 - 1);
		std::uniform_int_distribution<int> distY(0, kWindowHeight - 64);
		KamataEngine::Sprite* spriteZombie = nullptr;
		
		for (int i = 0; i < zombieCount; ++i) {
			spriteZombie = Sprite::Create(textureZombie_, {0, 0});
			enemies_.push_back(Enemy{static_cast<float>(distX(rng_)), y, redSpeed, 0,spriteZombie_});
			y += 50.0f;
			delete spriteZombie;
		}
		for (int i = 0; i < skeletonCount; ++i) {
			enemies_.push_back(Enemy{static_cast<float>(distX(rng_)), static_cast<float>(distY(rng_)), blueSpeed, 1,spriteZombie});
		}
		for (int i = 0; i < wolfCount; ++i) {
			enemies_.push_back(Enemy{static_cast<float>(distX(rng_)), static_cast<float>(distY(rng_)), graySpeed, 2, spriteZombie});
		}
		for (int i = 0; i < reaperCount; ++i) {
			enemies_.push_back(Enemy{static_cast<float>(distX(rng_)), static_cast<float>(distY(rng_)), yellowSpeed, 3, spriteZombie});
		}
		
	}

	void UpdateBallsDuringNight() {
		for (auto& b : enemies_) {
			b.x += b.speed;
			// 右端を超えたら簡易的に画面外へ送り出す（念のため）
			if (b.x > kWindowWidth + 64.0f) {
				b.x = -64.0f;
			}
		}
	}

	void MoveEnemies() { 
		for (auto& b : enemies_) {
			b.x += b.speed;
			b.sprite->SetPosition({b.x, b.y});
		}
	}

	void DrawEnemies() {
		// 64x64 の丸を各色で描画（ブラシを都度作って削除）
		for (const auto& b : enemies_) {
			/*Sprite* brush = nullptr;*/
			//switch (b.type) {
			//case 0:
			//	spriteZombie_->Draw();
			//	break; // ゾンビ
			//case 1:
			//	spriteSkeleton_->Draw();
			//	break; // スケルトン
			//case 2:
			//	spriteWerewolf_->Draw();
			//	break; // 狼男
			//case 3:
			//	spriteReaper_->Draw();
			//	break; // 死神
			//}
			/*HBRUSH old = (HBRUSH)SelectObject(hdc, brush);*/
			// Ellipse の矩形を 64x64 に調整
			/*Ellipse(hdc, (int)b.x, (int)b.y, (int)(b.x + 64.0f), (int)(b.y + 64.0f));
			SelectObject(hdc, old);*/
			/*DeleteObject(brush);*/
			b.sprite->Draw();
		}
	}

	// ---------- ゲーム本体 ----------
	void StartDay() {
		phase_ = TimePhase::Day;
		phaseTimer_ = 600;
		dayCount_++;
		points_ += lastEarnedPoints_;
		eventMessage_.clear();

		SpawnEnemies();
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

	void UpdatePlay() {
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

			if (Input::GetInstance()->TriggerKey(DIK_1) && points_ >= attackCost) {
				playerAttack_ += 2;
				points_ -= attackCost;
			}
			if (Input::GetInstance()->TriggerKey(DIK_2) && points_ >= defenseCost) {
				baseDefense_ += 2;
				points_ -= defenseCost;
			}
			if (Input::GetInstance()->TriggerKey(DIK_3) && points_ > 0) {
				baseHP_ += 10;
				if (baseHP_ > 200)
					baseHP_ = 200;
				points_ -= 1;
			}

			if (Input::GetInstance()->TriggerKey(DIK_R)) {
				points_ = dayPointsBackup_;
				playerAttack_ = attackBackup_;
				baseDefense_ = defenseBackup_;
				baseHP_ = baseHPBackup_;
			}

			if (Input::GetInstance()->TriggerKey(DIK_RETURN) || phaseTimer_ <= 0) {
				StartNight();
			}
			break;

		case TimePhase::Night:
			nightTimer_--;
			UpdateBallsDuringNight();
			MoveEnemies();
			if (nightTimer_ <= 0)
				StartResult();
			break;

		case TimePhase::Result:
			if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
				StartDay();
			}
			break;
		}
	}

	void DrawPlay() {
		switch (phase_) {
		case TimePhase::Day:
			spriteNoon_->Draw();
			spriteHeal_->Draw();
			spriteLife_->Draw();
			spriteSword_->Draw();
			spriteArmor_->Draw();
			spritePoint_->Draw();
			spriteAttack_->Draw();
			spriteDefense_->Draw();
			spritePlayer_->Draw();
			spriteStatus_->Draw();
			DrawDayNumber(dayCount_);
			DrawHPNumber(baseHP_);
			DrawAttackNumber(playerAttack_);
			DrawDefenseNumber(baseDefense_);
			DrawPointNumber(points_);
			DrawAttackCostNumber(playerAttack_ / 100 + 1);
			DrawDefenseCostNumber(baseDefense_ / 100 + 1);
			DrawHPCostNumber(1);
			break;

		case TimePhase::Night:
			spriteNight_->Draw();
			spritePlayer2_->Draw();
			DrawEnemies();
			break;

		case TimePhase::Result:
			spriteMorning_->Draw();
			spriteResult_->Draw();
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
			DrawEnemies();

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
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	Initialize(L"夜の番人");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//// ゲームシーンのインスタンス生成
	//GameScene* gameScene = new GameScene();
	//// ゲームシーンの初期化
	//gameScene->Initialize();
	Game game;
	game.LoadResources();
	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {
			break;
		}
		// ゲームシーンの更新
		game.Update();
		//
				
		// 描画開始
		dxCommon->PreDraw();
		Sprite::PreDraw();
		// ゲームシーンの描画
		game.Draw();
		Sprite::PostDraw();
		// 描画終了
		dxCommon->PostDraw();
	}
	// ゲームシーンの解放
	//delete gameScene;
	//// nullptrの代入
	//gameScene = nullptr;
	// エンジンの終了処理
	Finalize();
	delete spriteTitle_;
	delete spriteExplanation_;
	delete spriteNoon_;
	delete spriteNight_;
	delete spriteMorning_;
	delete spriteGameOver_;
	delete spritePlayer_;
	delete spritePlayer2_;
	delete spriteHeal_;
	delete spriteArmor_;
	/*delete spriteShield_;*/
	delete spriteSword_;
	delete spriteZombie_;
	delete spriteSkeleton_;
	delete spriteWerewolf_;
	delete spriteReaper_;
	return 0;
}

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
//	WNDCLASSEXW wc = {};
//	wc.cbSize = sizeof(WNDCLASSEXW);
//	wc.lpfnWndProc = WndProc;
//	wc.hInstance = hInstance;
//	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
//	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//	wc.lpszClassName = kWindowClassName;
//	RegisterClassExW(&wc);
//
//	RECT wr = {0, 0, kWindowWidth, kWindowHeight};
//	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
//	HWND hwnd = CreateWindowW(kWindowClassName, kWindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);
//
//	if (!hwnd)
//		return -1;
//
//	ShowWindow(hwnd, SW_SHOW);
//	UpdateWindow(hwnd);
//
//	HDC frontDC = GetDC(hwnd);
//	HDC backDC = CreateCompatibleDC(frontDC);
//	HBITMAP backBitmap = CreateCompatibleBitmap(frontDC, kWindowWidth, kWindowHeight);
//	HBITMAP oldBackBmp = (HBITMAP)SelectObject(backDC, backBitmap);
//
//	Game game;
//	game.LoadResources(hInstance, hwnd);
//
//	std::array<BYTE, 256> keys = {};
//	std::array<BYTE, 256> preKeys = {};
//
//	bool running = true;
//	MSG msg = {};
//
//	const int targetFPS = 60;
//	const int frameMs = 1000 / targetFPS;
//
//	while (running) {
//		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
//			if (msg.message == WM_QUIT)
//				running = false;
//			TranslateMessage(&msg);
//			DispatchMessageW(&msg);
//		}
//
//		preKeys = keys;
//		for (int i = 0; i < 256; ++i)
//			keys[i] = (GetAsyncKeyState(i) & 0x8000) ? 1 : 0;
//
//		if (keys[VK_ESCAPE] && !preKeys[VK_ESCAPE])
//			running = false;
//
//		game.Update(keys, preKeys);
//
//		game.Draw(backDC, kWindowWidth, kWindowHeight);
//
//		BitBlt(frontDC, 0, 0, kWindowWidth, kWindowHeight, backDC, 0, 0, SRCCOPY);
//
//		Sleep(frameMs);
//	}
//	SelectObject(backDC, oldBackBmp);
//	DeleteObject(backBitmap);
//	DeleteDC(backDC);
//	ReleaseDC(hwnd, frontDC);
//
//	DestroyWindow(hwnd);
//	return 0;
//}
