#include <bangtal>
#include <iostream>
#include <vector>
#include <map>

using namespace bangtal;
using namespace std;

bool pick_object(ObjectPtr object, int x, int y, MouseAction) {
	object->pick();
	return true;
}

class Move {
public:
	vector<vector <ScenePtr>> scenes_moving; //움직일 수 있는 장면
	int place = 0; //움직이려는 장면 (현재)
	int level = 0; //기본 장면은 0, 그 장면만 연결되는 장면은 1
	bool can_move = true;

	TimerPtr walk_timer = Timer::create(0.03f);
	TimerPtr jump_timer = Timer::create(0.01f);
	int me_x = 0, me_y = 22;
	int walk_speed = 20;
	int jump_speed = 10; int fall_speed = 10;
	int jumped = 0;
	bool jumping = false;
	bool fall = false;
};

class Item {
public:
	ScenePtr scene;
	Item(ScenePtr s): scene(s){}

	ObjectPtr beans = Object::create("Images/beans.png", scene, 381, 144); bool beans_picked = false;
	ObjectPtr shovel = Object::create("Images/shovel.png", scene, 721, 98); bool shovel_picked = false;
	ObjectPtr bucket = Object::create("Images/bucket_empty.png", scene, 823, 267); bool bucket_picked = false;
	bool bucket_state = false; //처음엔 빈 양동이

	void itemPick(Move m) {
		beans->setOnMouseCallback([&](ObjectPtr object, int, int, MouseAction)->bool {
			object->pick();
			beans_picked = true;
			return true;
			});
		shovel->setOnMouseCallback([&](ObjectPtr object, int, int, MouseAction)->bool {
			object->pick();
			shovel_picked = true;
			return true;
			});
		bucket->setOnMouseCallback([&](ObjectPtr object, int, int, MouseAction)->bool {
			if (m.place == 2) { if (bucket_state) object->pick(); }
			else { object->pick(); bucket_picked = true; }
			return true;
			});
	}
};

class Plant {
private:
	ScenePtr scene;
	TimerPtr plant_timer = Timer::create(0.01f);

	int plant_y = -922;
	int grown = 0;

	ObjectPtr plant = Object::create("Images/plant.png", scene, 570, plant_y);
	ObjectPtr sand_planted = Object::create("Images/sand_pile.png", scene, 0, 2, false);
	ObjectPtr sand_floor = Object::create("Images/front_yard_sand.png", scene, 0, 0);
	ObjectPtr plant_guide = Object::create("Images/plant_guide.png", scene, 297, 44);

	ObjectPtr hole = Object::create("Images/hole.png", scene, 0, 0, false);
	ObjectPtr bean = Object::create("Images/bean.png", scene, 0, -3, false);
	ObjectPtr sand_pile = Object::create("Images/sand_pile.png", scene, 0, 42, false);

public:
	int clicked = 0;
	enum class Garden { NONE, HOLE, BEAN, PLANTED, GROW1, GROW2, GROW3, GROW4 };
	Garden garden_state = Garden::NONE;
	Plant(ScenePtr s): scene(s){}

	//콩 키우기
	void seeGuide() {
		plant_guide->setOnMouseCallback([](ObjectPtr, int, int, MouseAction)->bool {
			showImageViewer("Images/howtoplant.png");
			return true;
		});
	}
	void growBean(ObjectPtr beans,ObjectPtr shovel, ObjectPtr bucket,bool& bucket_state) {
		sand_floor->setOnMouseCallback([&](ObjectPtr o, int x, int y, MouseAction)->bool {
			if (garden_state == Garden::NONE && shovel->isHanded() && x > 350 && x < 1160) {
				clicked = x;
				hole->locate(scene, clicked - 50, 5); hole->show();
				sand_pile->locate(scene, clicked + 45, 42); sand_pile->show();
				sand_pile->setOnMouseCallback(pick_object);
				plant->locate(scene, clicked - 157, plant_y);
				garden_state = Garden::HOLE;
				hole->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
					if (beans->isHanded()) {
						beans->drop(); beans->hide();
						bean->locate(scene, clicked - 25, -3); bean->show();
						garden_state = Garden::BEAN;
						bean->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
							if (sand_pile->isHanded()) {
								hole->hide(); bean->hide(); sand_pile->drop(); sand_pile->hide();
								sand_planted->locate(scene, clicked - 75, 2); sand_planted->show();
								garden_state = Garden::PLANTED;
								sand_planted->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
									if ((garden_state == Garden::PLANTED || garden_state == Garden::GROW1 || garden_state == Garden::GROW2 || garden_state == Garden::GROW3) && bucket->isHanded() && bucket_state) {
										plant_timer->start();
										bucket->setImage("Images/bucket_empty.png"); bucket_state = false;
										switch (garden_state) {
										case Garden::PLANTED: garden_state = Garden::GROW1; break;
										case Garden::GROW1: garden_state = Garden::GROW2; break;
										case Garden::GROW2:garden_state = Garden::GROW3; break;
										case Garden::GROW3: garden_state = Garden::GROW4; break;
										}
									}
									plant_timer->setOnTimerCallback([&](TimerPtr t)->bool { //196
										plant_y += 7;  grown++;
										plant->locate(scene, clicked - 157, plant_y);
										if (grown < 28) { t->set(0.01f); t->start(); }
										else { t->set(0.01f); grown = 0; }
										return true;
										});
									return true;
									});
							}
							return true;
							});
					}
					return true;
				});
			}
			return true;
		});
	}
};

class Well {
private:
	ScenePtr scene;
	TimerPtr well_timer = Timer::create(2.0f);
	ObjectPtr well = Object::create("Images/well.png", scene, 419, 8);
	ObjectPtr meter = Object::create("Images/meter.png", scene, 743, 27);
	bool meter_on = false;
	ObjectPtr w_npc = Object::create("Images/well_npc1.png", scene, 805, -21);
	TimerPtr w_npc_timer = Timer::create(0.3f);

public:
	Well(ScenePtr s) :scene(s) {}
	void npcMove() {
		w_npc_timer->setOnTimerCallback([=](TimerPtr t)->bool {
			if (rand() % 2) w_npc->setImage("Images/well_npc2.png");
			else w_npc->setImage("Images/well_npc1.png");
			t->set(0.3f); t->start();
			return true;
			});
		w_npc_timer->start();
		w_npc->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
			showMessage("우물 어떻게 쓰냐고?\n장치에 토큰 넣고...우물에 양동이 연결하든가");
			return true;
			});
	}
	void useMeter(const ObjectPtr tokens[]) {
		meter->setOnMouseCallback([&, tokens](ObjectPtr, int, int, MouseAction)->bool {
			for (int i = 0; i < 4; i++) {
				if (!meter_on && tokens[i]->isHanded()) {
					meter->setImage("Images/meter_on.png");
					tokens[i]->drop();
					tokens[i]->hide();
					meter_on = true;
				}
			}
			return true;
		});
	}
	void useWell(const ObjectPtr bucket,bool& bucket_state) {
		well->setOnMouseCallback([&, bucket](ObjectPtr, int, int, MouseAction)->bool {
			if (meter_on && !bucket_state && bucket->isHanded()) {
				bucket->drop();
				bucket->locate(scene, 550, 198);
				well_timer->setOnTimerCallback([&](TimerPtr t)->bool {
					bucket->setImage("Images/bucket_full.png");
					bucket_state = true;
					meter->setImage("Images/meter.png");
					meter_on = false;
					t->set(2.0f);
					return true;
					});
				well_timer->start();
			}
			return true;
		});
	}
};

class Pullrope {
public:
	int me_x = 230, me_y = 160;
	int npc_x = 850, npc_y = 160;
	int rope_x = 261, rope_y = 198;
	int sign_y = 228;
	int f_speed = 10;
	bool pullrope_win = false;

	ScenePtr pullrope_scene = Scene::create("", "Images/pullrope_scene.png");
	ObjectPtr rope = Object::create("Images/rope.png", pullrope_scene, rope_x, rope_y);
	ObjectPtr me = Object::create("Images/spongebob_right.png", pullrope_scene, me_x, me_y);
	ObjectPtr npc = Object::create("Images/lobster_g.png", pullrope_scene, npc_x, npc_y);

	ObjectPtr start_sign = Object::create("Images/pullrope_sign.png", pullrope_scene, 176, sign_y);
	ObjectPtr number = Object::create("Images/3.png", pullrope_scene, 585, 338, false); int num = 3;
	ObjectPtr result = Object::create("Images/you_win.png", pullrope_scene, 365, 509, false);
	ObjectPtr go_back = Object::create("Images/go_back.png", pullrope_scene, 426, 383, false);
	ObjectPtr try_again = Object::create("Images/try_again.png", pullrope_scene, 426, 294, false);

	TimerPtr npc_timer = Timer::create(0.1f);
	TimerPtr fall_timer = Timer::create(0.01f);
	TimerPtr num_timer = Timer::create(1.0f);
	TimerPtr sign_timer = Timer::create(0.01f);

	void signMove() {
		start_sign->setOnMouseCallback([&](ObjectPtr object, int x, int y, MouseAction)->bool {
			if (x > 240 && x < 700 && y>72 && y < 121) { //press to start
				sign_timer->setOnTimerCallback([&](TimerPtr t)->bool {
					if (sign_y < 800) {
						sign_y += 5;
						start_sign->locate(pullrope_scene, 176, sign_y);
						t->set(0.01f); t->start();
					}
					else { //표지판 다 올라가면 3,2,1 카운트 다운 후 게임 시작
						sign_timer->set(0.01f);
						number->setImage("Images/3.png"); num = 3;
						number->show();
						num_timer->setOnTimerCallback([&](TimerPtr t)->bool {
							num--;
							switch (num) {
							case(2): number->setImage("Images/2.png"); break;
							case(1):number->setImage("Images/1.png"); break;
							}
							if (num > 0) { t->set(1.0f); t->start(); }
							else {
								number->hide();
								npc_timer->start();
								me->setOnMouseCallback([&](ObjectPtr o, int x, int y, MouseAction)->bool { //내 캐릭터 클릭하면 줄 당김
									int speed = 20;
									if (me_y > 159 && npc_y > 159) {
										me_x -= speed; npc_x -= speed; rope_x -= speed;
										me->locate(pullrope_scene, me_x, me_y); npc->locate(pullrope_scene,npc_x, npc_y); rope->locate(pullrope_scene, rope_x, rope_y);
									}
									return true;
									});
								t->set(1.0f);
							}
							return true;
							});
						num_timer->start();
					}
					return true;
					});
				sign_timer->start();
			}
			return true;
			});
	}

	void start() {
		npc_timer->setOnTimerCallback([&](TimerPtr t)->bool {
			const int speed = 10;
			me_x += speed; npc_x += speed; rope_x += speed;
			me->locate(pullrope_scene, me_x, me_y); npc->locate(pullrope_scene, npc_x, npc_y); rope->locate(pullrope_scene, rope_x, rope_y);
			if (me_x > 450 || npc_x < 595) {
				if (me_x > 450) {  //내가 짐
					fall_timer->setOnTimerCallback([&](TimerPtr t1)->bool {
						me_y -= f_speed;
						me->locate(pullrope_scene, me_x, me_y);
						if (me_y > -250) {
							t1->set(0.01f);
							t1->start();
						}
						else { //내가 지면 다시 시작, 나가기 선택 가능
							result->setImage("Images/you_lose.png");
							result->show();
							go_back->show();
							try_again->show();
							try_again->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
								result->hide(); go_back->hide(); try_again->hide();
								me_x = 230, me_y = 160; me->locate(pullrope_scene, me_x, me_y);
								npc_x = 850, npc_y = 160; npc->locate(pullrope_scene, npc_x, npc_y);
								rope_x = 261; rope->locate(pullrope_scene,rope_x,rope_y);
								sign_timer->start();
								npc_timer->set(0.1f);  //게임 다시 시작
								return true;
								});
						}
						return true;
						});
					fall_timer->start();
				}
				else if (npc_x < 595) { //내가 이김
					fall_timer->setOnTimerCallback([&](TimerPtr t1)->bool {
						npc_y -=f_speed;
						npc->locate(pullrope_scene, npc_x, npc_y);
						if (npc_y > -350) { t1->set(0.01f); t1->start(); }
						else { //내가 이기면 나가기만 선택 가능
							result->show();
							go_back->show();
							result->setImage("Images/you_win.png");
							pullrope_win = true;
						}
						return true;
						});
					fall_timer->start();
				}

			}
			else { t->set(0.1f); t->start(); }
			result->hide();
			go_back->hide();
			return true;
		});
	}

};

class PicMatch {
private:
	ObjectPtr cards[3][8]; //카드 12쌍
	map<ObjectPtr, int> m;
	const int start_x = 111, diff_x = 136, start_y = 76, diff_y = 197;

	int t = 0;
	int flipped = 0;
	int correct = 0;
	int first_flip = 0, second_flip = 0;
	ObjectPtr p_first_flip, p_second_flip;
	TimerPtr flip_timer = Timer::create(0.5f);

public:
	ScenePtr picmatch_scene = Scene::create("", "Images/picmatch_scene.png");
	ScenePtr picmatch_t = Scene::create("","Images/picmatch_scene.png");
	bool picmatch_win = false;
	
	ObjectPtr result = Object::create("Images/you_win.png", picmatch_t, 365, 509, false);
	ObjectPtr go_back = Object::create("Images/go_back.png", picmatch_t, 426, 383, false);
	ObjectPtr try_again = Object::create("Images/try_again.png", picmatch_t, 426, 294, false);
	ObjectPtr picmatch_howto = Object::create("Images/picmatch_howto.png", picmatch_t, 0, 0);
	TimerPtr play_timer = Timer::create(50.0f);

	void makeCard() {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 8; j++) {
				cards[i][j] = Object::create("Images/card_back.png", picmatch_scene, start_x + diff_x * j, start_y + diff_y * i);
			}
		}
	}
	void setup() {
		for (int i = 0; i < 3; i++) { //카드 뒷면 보이게
			for (int j = 0; j < 8; j++) {
				cards[i][j]->setImage("Images/card_back.png");
				cards[i][j]->show();
			}
		}
		m.clear(); //전 게임 카드 조합 삭제
		t=flipped=correct=first_flip=second_flip = 0;
		p_first_flip=p_second_flip = NULL;
		flip_timer->set(0.5f);
		play_timer->set(50.0f);
	}
	void play() {
		vector<int> types{ 1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12 };
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 8; j++) {
				t = types[rand() % (24 - i * 8 - j)];
				types.erase(find(types.begin(), types.end(), t)); //카드 섞기
				m.insert(make_pair(cards[i][j], t)); //type 저장
				cards[i][j]->setOnMouseCallback([&](ObjectPtr object, int, int, MouseAction)->bool {
						if (flipped<2 && object != p_first_flip) {
							flipped++;
							string filename = "Images/card" + to_string(m.at(object)) + ".png";
							object->setImage(filename); //카드 뒤집음
							if (flipped == 1) {
								first_flip = m.at(object);
								p_first_flip = object;
							}
							else { //flipped==2
								second_flip = m.at(object);
								flip_timer->setOnTimerCallback([=](TimerPtr t)->bool {
									if (first_flip != second_flip) { //틀림
										p_first_flip->setImage("Images/card_back.png");
										object->setImage("Images/card_back.png");
										p_first_flip = NULL;
									}
									else { //정답
										correct++;
										p_first_flip->hide();
										object->hide();
										if (correct == 12) { //성공
											hideTimer();
											play_timer->stop();
											picmatch_t->enter();
											result->show();
											go_back->show();
											try_again->hide();
											result->setImage("Images/you_win.png");
											picmatch_win = true;
										}
									}
									flipped = 0;
									t->set(0.5f);
									return true;
									});
								flip_timer->start();
							}
						}
					
					return true;
				});
				
			}
		}
	}
};

class Hanoi {
public:
	ScenePtr hanoi_scene = Scene::create("", "Images/hanoi_scene.png");
	ScenePtr hanoi_t = Scene::create("", "Images/blank.png");
	bool hanoi_win = false;

	ObjectPtr result = Object::create("Images/you_win.png", hanoi_scene, 365, 509, false);
	ObjectPtr go_back = Object::create("Images/go_back.png", hanoi_scene, 426, 383, false);
	ObjectPtr hanoi_howto = Object::create("Images/hanoi_howto.png", hanoi_t, 0, 0);
private:
	ObjectPtr nate = Object::create("Images/nate.png", hanoi_scene, 846, -143);
	ObjectPtr counter = Object::create("Images/counter.png", hanoi_scene, 0, 0);
	ObjectPtr order = Object::create("Images/order.png", hanoi_scene, 13, 569);

	ObjectPtr dish1 = Object::create("Images/dish.png", hanoi_scene, 181, 0);
	ObjectPtr dish2 = Object::create("Images/dish.png", hanoi_scene, 518, 0);
	ObjectPtr dish3 = Object::create("Images/dish.png", hanoi_scene, 855, 0);
	vector<ObjectPtr> dishes{ dish1,dish2,dish3 };

	ObjectPtr blocks[5]; //0: bottom bun, 1: patty, 2: tomato, 3: lettuce, 4: top bun

	int place_x[3] = { 189,526,863 }; //각 블럭의 위치별 x좌표 place_x[블럭종류][막대 위치]
	int place_y[5] = { 61,111,161,211,261 }; //모든 블럭의 공통 y좌표

	int floated_y = 538;
	bool floated = false;

	vector<vector<int>> state{ {0,1,2,3,4},{},{} }; //각 막대별로 꽂혀있는 블럭
	int place[5] = { 0,0,0,0,0 }; //각 블럭의 현재 막대 위치 

public:
	void play() { 
		for (int i = 0; i < 5; i++) {
			string filename = "Images/block" + to_string(i + 1) + ".png";
			blocks[i] = Object::create(filename, hanoi_scene, place_x[0], place_y[i]);
			blocks[i]->setOnMouseCallback([&, i](ObjectPtr, int, int, MouseAction)->bool {
				if (!floated && *(state[place[i]].end() - 1) == i) {
					blocks[i]->locate(hanoi_scene, place_x[place[i]], floated_y);
					state[place[i]].erase(state[place[i]].end() - 1);
					floated = true;
					for (int j = 0; j < 3; j++) {
						dishes[j]->setOnMouseCallback([&, i, j](ObjectPtr, int, int, MouseAction)->bool {
							if (floated && (state[j].size() == 0 || i > * (state[j].end() - 1))) { //큰 블럭 위, 또는 빈 막대에만 놓을 수 있음
								state[j].push_back(i);
								place[i] = j;
								blocks[i]->locate(hanoi_scene, place_x[j], place_y[state[j].size() - 1]);
								floated = false;
								if (j == 2 && state[j].size() == 5) {
									nate->setImage("Images/nate_happy.png");
									hanoi_win = true;
									result->show();
									go_back->show();
								}
							}
							return true;
							});
					}
				}
				return true;
				});


		}
	}
};

class Worm {
public:
	ScenePtr worm_scene = Scene::create("", "Images/worm_scene1.png");
	ScenePtr worm_t = Scene::create("", "Images/worm_scene1.png");
	bool worm_win = false;

	ObjectPtr result = Object::create("Images/you_win.png",worm_t, 365, 509, false);
	ObjectPtr try_again = Object::create("Images/try_again.png", worm_t, 426, 294, false);
	ObjectPtr go_back = Object::create("Images/go_back.png", worm_t, 426, 383, false);
	ObjectPtr worm_howto = Object::create("Images/worm_howto.png", worm_t, 0, 0);
	TimerPtr scoreboard = Timer::create(score);
private:
	bool gameOver = false;

	int head_x = 0, head_y = 0, fruit_x = 0, fruit_y = 0;
	float score = 0;

	const int width = 32;
	const int height = 18;
	const int diff = 40; //40 pixels

	vector <ObjectPtr> worm{};

	vector <vector<int>> xy{};

	ObjectPtr head = Object::create("Images/head_down.png", worm_scene, diff * (width / 2), diff * (height / 2));
	ObjectPtr temp = NULL;
	int new_x = 0, new_y = 0;

	const float speed = 0.1f;
	TimerPtr move_timer = Timer::create(speed);

	ObjectPtr fruit = Object::create("Images/pineapple.png", worm_scene, 0, 0, false);
	bool ate = false;

	enum class Direction { STOP, LEFT, RIGHT, UP, DOWN };
	Direction dir = Direction::STOP;

public:
	void setup() {
		gameOver = false;
		dir = Direction::STOP;
		ate = false;

		head_x = width / 2; //0~31
		head_y = height / 2; //0~17
		head->setImage("Images/head_down.png");

		do {
			fruit_x = rand() % width;
			fruit_y = rand() % height;
		} while (fruit_x==head_x); //처음은 같은 줄에 안나타나게
		fruit->locate(worm_scene, diff * fruit_x, diff * fruit_y);
		score = 0;
		scoreboard->set(score);
	}

	bool check(int x, int y) {
		for (int i = 1; i < worm.size(); i++) {
			if (x == xy[i][0] && y == xy[i][1]) {
				return true;
			}
		}
		return false;
	}

	void run() {
		for (int i = 1; i < worm.size(); i++) {
			worm[i]->hide();
		}
		worm.erase(worm.begin(), worm.end());
		xy.erase(xy.begin(), xy.end());

		worm.push_back(head);
		xy.push_back({ head_x,head_y });

		dir = Direction::DOWN;
		move_timer->start();
		fruit->show();
		move_timer->setOnTimerCallback([&](TimerPtr t)->bool {
			//머리 제외 몸통 부분 뒤에서부터 좌표 변경
			if (worm.size() > 1) {
				if (ate) { //몸통 추가할 좌표, 일단 만들어놓고 다음 움직일 때 worm에 추가
					new_x = xy[worm.size() - 1][0];
					new_y = xy[worm.size() - 1][1];
				}
				for (int i = worm.size() - 1; i > 0; i--) {
					xy[i][0] = xy[i - 1][0]; //앞 몸이 움직인 좌표로 이동
					xy[i][1] = xy[i - 1][1];
				}
			}
			//머리 좌표 변경
			switch (dir) {
			case Direction::LEFT: if (xy[0][0] == 0) xy[0][0] = 31; else xy[0][0]--; break;
			case Direction::RIGHT: if (xy[0][0] == 31) xy[0][0] = 0; else xy[0][0]++; break;
			case Direction::UP: if (xy[0][1] == 17) xy[0][1] = 0; else xy[0][1]++; break;
			case Direction::DOWN: if (xy[0][1] == 0) xy[0][1] = 17; else xy[0][1]--; break;
			}

			//꼬리에 닿으면 gameover
			if (check(xy[0][0], xy[0][1])) {
				gameOver = true;
				dir = Direction::STOP;
				string message = to_string(int(score)) + "점을 달성했습니다!";
				showMessage(message);
				if (score > 39) { //성공
					result->setImage("Images/you_win.png");
					worm_win = true;
				}
				else { //실패
					result->setImage("Images/you_lose.png");
					try_again->show();
				}
				result->show(); go_back->show(); worm_howto->hide();
				worm_t->enter();
				return true;
			}

			//바뀐 좌표에 따라 움직임
			for (int i = 0; i < worm.size(); i++) {
				worm[i]->locate(worm_scene, diff * xy[i][0], diff * xy[i][1]);
			}

			//만든 몸통 추가
			if (ate) {
				worm.push_back(temp);
				xy.push_back({ new_x,new_y });
				ate = false;
			}
			//과일 먹으면 몸통 한 개씩 추가
			if (xy[0][0] == fruit_x && xy[0][1] == fruit_y) {
				scoreboard->set(++score);
				ate = true;

				temp = Object::create("Images/body.png", worm_scene, diff * xy[worm.size() - 1][0], diff * xy[worm.size() - 1][1]);
				do {
					fruit_x = rand() % width;
					fruit_y = rand() % height;
				} while (check(fruit_x, fruit_y) || (xy[0][0] == fruit_x && xy[0][1] == fruit_y)); //머리+몸통 있는 곳엔 과일 안 생기게
				fruit->locate(worm_scene, diff * fruit_x, diff * fruit_y);
			}
			//gameOver되면 움직임 멈춤
			if (dir != Direction::STOP) {
				t->set(speed);
				t->start();
			}
			return true;
			});

	}
	void control() {
		worm_scene->setOnKeyboardCallback([&](ScenePtr, int key, bool pressed)->bool {
			if (!gameOver && pressed) {
				if (key == 1 && dir != Direction::RIGHT) { //왼쪽
					dir = Direction::LEFT;
					head->setImage("Images/head_left.png");
				}
				else if (key == 4 && dir != Direction::LEFT) { //오른쪽
					dir = Direction::RIGHT;
					head->setImage("Images/head_right.png");
				}
				else if (key == 23 && dir != Direction::DOWN) { //위
					dir = Direction::UP;
					head->setImage("Images/head_up.png");
				}
				else if (key == 19 && dir != Direction::UP) { //아래
					dir = Direction::DOWN;
					head->setImage("Images/head_down.png");
				}
			}
			return true;
			});
	}
};

int main() {
	setGameOption(GameOption::GAME_OPTION_ROOM_TITLE, false);
	setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
	setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);

	Move m;
	//TITLE
	auto title = Scene::create("", "Images/title.png");
	auto start = Object::create("Images/game_start.png", title, 661, 185);
	start->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
		m.scenes_moving[0][0]->enter();
		setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, true);
		return true;
	});
	//장면0: 집
	auto my_house = Scene::create("", "Images/my_house.png"); 
	auto sign = Object::create("Images/sign.png", my_house, 437, 427);
	m.scenes_moving.push_back({ my_house });
	sign->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
		showImageViewer("Images/game_instructions.png");
		return true;
	});
	Item item(my_house);
	item.itemPick(m); 

	//장면1: 앞마당
	auto front_yard = Scene::create("", "Images/front_yard.png");
	Plant pl(front_yard);
	pl.seeGuide();
	pl.growBean(item.beans, item.shovel, item.bucket, item.bucket_state);
	
	//장면 1-1: 콩 줄기 위 배 
	auto treasure_scene = Scene::create("", "Images/treasure_scene.png");
	auto treasure = Object::create("Images/button.png", treasure_scene, 853, 0);
	m.scenes_moving.push_back({ front_yard,treasure_scene });
	treasure->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
		showImageViewer("Images/a+.png");
		showMessage("축하합니다! 당신은 A+ 빔을 맞았습니다!\n (게임을 종료하려면 ESC 키를 누르세요)");
		return true;
		});

	//장면2 : 우물
	auto well_scene = Scene::create("", "Images/well_scene.png");
	m.scenes_moving.push_back({well_scene});

	//장면3: 줄달리기
	Pullrope p;
	auto lobster_scene = Scene::create("", "Images/lobster_scene.png");
	m.scenes_moving.push_back({ lobster_scene});
	ObjectPtr token1 = Object::create("Images/coin.png",lobster_scene, 400, 50,false);
	auto lobster = Object::create("Images/lobster.png", lobster_scene, 500, 6);
	lobster->setOnMouseCallback([&](ObjectPtr o, int x, int y, MouseAction)->bool {
		if (!p.pullrope_win) {
			showMessage("나랑 줄달리기 대결 해볼래?\n (대결을 승낙하려면 ENTER를 누르세요)");
			p.signMove();
			p.start();
			p.go_back->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool { //랍스터 scene으로 다시 이동
				setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, true);
				lobster_scene->enter();
				if (p.pullrope_win) {
					showMessage("너가 이겼으니까 이거 줄게\n 우물 사용하는 건 알지?");
					token1->show();
				}
				return true;
			});
			
		}
		else { showMessage("조금 센데?"); }
		return true;
	});

	//장면4: 그림 맞추기
	PicMatch pic;
	auto squidward_scene = Scene::create("", "Images/squidward_scene.png");
	m.scenes_moving.push_back({ squidward_scene });
	auto squidward = Object::create("Images/squidward.png", squidward_scene, 743, 17);
	ObjectPtr token2 = Object::create("Images/coin.png", squidward_scene, 500, 50, false);
	pic.makeCard();
	squidward->setOnMouseCallback([&](ObjectPtr o, int x, int y, MouseAction)->bool {
		if (!pic.picmatch_win) {
			showMessage("나보다 기억력이 좋아? 흥 그럴리 없지\n (대결을 승낙하려면 ENTER를 누르세요)");
			pic.picmatch_howto->setOnMouseCallback([&](ObjectPtr, int, int,MouseAction)->bool {
				pic.picmatch_howto->hide();
				pic.picmatch_scene->enter();
				showTimer(pic.play_timer);
				pic.setup();
				pic.play();
				pic.play_timer->start();
				pic.play_timer->setOnTimerCallback([&](TimerPtr t)->bool { //실패
					hideTimer();
					pic.picmatch_t->enter(); 
					pic.result->setImage("Images/you_lose.png");
					pic.result->show();
					pic.go_back->show();
					pic.try_again->show();
					pic.try_again->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
						showTimer(pic.play_timer);
						p.result->hide(); p.go_back->hide(); p.try_again->hide();
						pic.picmatch_scene->enter();
						pic.setup();
						pic.play();
						pic.play_timer->start();
						return true;
						});
					return true;
					});
				return true;
				});
			pic.go_back->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool { //랍스터 scene으로 다시 이동
				setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, true);
				pic.result->hide();
				pic.go_back->hide();
				pic.try_again->hide();
				squidward_scene->enter();
				if (pic.picmatch_win) {
					showMessage("조금 하네? 자, 이거 줄게");
					token2->show();
					}
				return true;
			});
						
				
		}
		else showMessage("아까 대결했잖아 스포온지밥");
		return true;
	});

	//장면 5: 하노이 탑
	Hanoi h;
	auto mrkrabs_scene = Scene::create("", "Images/mrkrabs_scene.png");
	m.scenes_moving.push_back({ mrkrabs_scene});
	auto mrkrabs = Object::create("Images/mrkrabs.png", mrkrabs_scene, 461, 26);
	ObjectPtr token3 = Object::create("Images/coin.png", mrkrabs_scene, 300, 25, false);
	mrkrabs->setOnMouseCallback([&](ObjectPtr o, int x, int y, MouseAction)->bool {
		if (!h.hanoi_win) {
			showMessage("스폰지이바압!!!! 출근 안 하고 뭐 해??!!!\n (일을 시작하려면 ENTER를 누르세요)");
			h.hanoi_howto->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
				h.hanoi_howto->hide();
				h.hanoi_scene->enter();
				return true;
				});
			h.go_back->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool { //랍스터 scene으로 다시 이동
				setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, true);
				h.result->hide();
				h.go_back->hide();
				mrkrabs_scene->enter();
				showMessage("수고했어 스폰지밥 이건 오늘 일당");
				token3->show();
				return true;
				});
		}
		else showMessage("오늘 이미 일했잖아! 돈 더 못 줘!");
		
		return true;
	});

	//장면6: 지렁이 게임 
	Worm w;
	auto mrspuff_scene = Scene::create("", "Images/mrspuff_scene.png");
	m.scenes_moving.push_back({ mrspuff_scene });
	auto mrspuff = Object::create("Images/mrspuff.png", mrspuff_scene, 653, 24);
	ObjectPtr token4 = Object::create("Images/coin.png", mrspuff_scene, 500, 25, false);
	mrspuff->setOnMouseCallback([&](ObjectPtr o, int x, int y, MouseAction)->bool {
		if (!w.worm_win) {
			showMessage("스폰지밥..? 운전 연습하러 왔어?\n(운전을 시작하려면 ENTER를 누르세요)");
			w.worm_howto->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
				w.worm_howto->hide();
				w.worm_scene->enter();
				showTimer(w.scoreboard);
				w.setup();
				w.run();
				w.control();
				w.try_again->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool {
					w.result->hide(); w.go_back->hide(); w.try_again->hide();
					w.worm_scene->enter();
					w.setup();
					w.run();
					return true;
					});
				return true;
				});
			w.go_back->setOnMouseCallback([&](ObjectPtr, int, int, MouseAction)->bool { //랍스터 scene으로 다시 이동
				setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, true);
				w.result->hide();
				w.try_again->hide();
				w.go_back->hide();
				hideTimer();
				mrspuff_scene->enter();
				if (w.worm_win) {
					showMessage("오호 운전 많이 늘었네?");
					token4->show();
				}
				return true;
				});
		}
		else showMessage("운전 자주해도 안 좋아");
		return true;
	});

	//토큰으로 우물 조작
	Well well(well_scene);
	ObjectPtr tokens[4]={ token1,token2,token3,token4 };
	for (auto o : tokens) o->setOnMouseCallback(pick_object);
	well.npcMove();
	well.useMeter(tokens);
	well.useWell(item.bucket, item.bucket_state);
	
	//스폰지밥 움직임 조작
	auto me = Object::create("Images/spongebob_right.png", my_house, 0, 22);
	for (int i = 0; i < m.scenes_moving.size(); ++i) { //캐릭터 움직임
		for (int j = 0; j < m.scenes_moving[i].size(); ++j){
			m.scenes_moving[i][j]->setOnKeyboardCallback([&, i](ScenePtr s, int key, bool pressed)->bool {
				if (m.can_move) {
					if (key == 1 && pressed && m.me_x > -20) { //왼쪽 이동
						m.walk_timer->setOnTimerCallback([&](TimerPtr t)->bool {
							m.me_x -= m.walk_speed;
							me->locate(m.scenes_moving[m.place][m.level], m.me_x, m.me_y);
							if (m.me_x > -40) { t->set(0.03f); t->start(); }
							if (m.me_x < -10 && m.level == 0 && m.place > 0) {  //이전 장면으로 이동
								m.place--; m.scenes_moving[m.place][m.level]->enter();
								m.me_x = 1000;
								t->set(0.03f); t->start();
							}
							if (pl.garden_state == Plant::Garden::GROW4 && m.place == 1 && m.me_x < pl.clicked - 162 && m.me_y > 91) { //z
								m.fall_speed = 30;
								m.jump_timer->start();
							}
							return true;
							});
						me->setImage("Images/spongebob_left.png");
						m.walk_timer->start();
					}
					else if (key == 1 && !pressed) { //손 때면 멈춤
						m.walk_timer->stop();
						m.walk_timer->set(0.03f);
					}
					else if (key == 4 && pressed && m.me_x < 1010) {  //오른쪽 이동
						m.walk_timer->setOnTimerCallback([&](TimerPtr t)->bool {
							m.me_x += m.walk_speed;
							me->locate(m.scenes_moving[m.place][m.level], m.me_x, m.me_y);
							if (m.me_x < 1010) {
								t->set(0.03f); t->start();
							}
							if (m.me_x > 1000 && m.level == 0 && m.place < m.scenes_moving.size() - 1) {  //다음 장면으로 이동
								if (m.place == 0 && (!(item.beans_picked) || !(item.shovel_picked) || !(item.bucket_picked))) showMessage("씨앗이랑 도구는 챙겨야지");
								else {
									m.place++; m.scenes_moving[m.place][m.level]->enter();
									m.me_x = -20;
									t->set(0.03f); t->start();
								}
							}
							if (pl.garden_state == Plant::Garden::GROW4 && m.place == 1 && m.me_x > pl.clicked-12 && m.me_y > 91) { //콩 줄기 위에서 떨어짐
								m.fall_speed = 30;
								m.jump_timer->start();
							}
							return true;
							});
						me->setImage("Images/spongebob_right.png");
						m.walk_timer->start();
					}
					else if (key == 4 && !pressed) { //손 때면 멈춤
						m.walk_timer->stop();
						m.walk_timer->set(0.03f);
					}
					else if (key == 23 && pressed && !m.jumping) { // 뛰기
						if (m.place == 1 && m.me_y > 720) { //콩 줄기 올라가면 boat scene 입장 
							m.level = 1;
							m.scenes_moving[m.place][1]->enter();
							m.me_x = 0; m.me_y = 22;
							me->locate(m.scenes_moving[m.place][1], m.me_x, m.me_y);
						}
						m.jump_timer->setOnTimerCallback([&](TimerPtr t)->bool {
							if (m.jumped < 12) m.me_y += m.jump_speed;  //뜀
							else { m.me_y -= m.fall_speed; } //떨어짐 

							if (m.me_y < 22) { m.me_y = 22; m.jumping = false; t->set(0.01f); } //땅위에 착지
							else {
								if (pl.garden_state == Plant::Garden::GROW4 && m.place == 1 && m.level == 0 && m.me_x > pl.clicked - 162 && m.me_x < pl.clicked-12 && m.jumped == 16) { m.jumping = false; t->set(0.01f); } //콩 줄기 점프해서 올라감
								else { t->set(0.01f); t->start(); }
							}
							m.jumped++;
							me->locate(m.scenes_moving[m.place][m.level], m.me_x, m.me_y);
							return true;
							});
						m.jumped = 0;
						m.jumping = true;
						m.fall_speed = 10;
						m.jump_timer->start();
					}
					else if (key == 67 && pressed) { 
						if (i == 3 && !p.pullrope_win) { //줄달리기 scene 입장
							setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
							p.pullrope_scene->enter(); 
							p.result->hide(); p.go_back->hide(); p.try_again->hide();
							p.me->setScale(0.7f);
							p.me_x = 230, p.me_y = 160; p.me->locate(p.pullrope_scene, p.me_x, p.me_y);

							p.npc_x = 850, p.npc_y = 160; p.npc->locate(p.pullrope_scene, p.npc_x, p.npc_y);
							p.rope_x = 261; p.rope->locate(p.pullrope_scene, p.rope_x, p.rope_y);
							p.sign_y = 228; p.start_sign->locate(p.pullrope_scene, 176, p.sign_y);
						}
						else if (i == 4 && !pic.picmatch_win) {
							setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
							pic.picmatch_t->enter(); 
							pic.picmatch_howto->show();
							pic.play();
						}
						else if (i == 5 && !h.hanoi_win) {
							setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
							h.hanoi_t->enter(); 
							h.hanoi_howto->show();
							h.play();
						}
						else if (i == 6 && !w.worm_win) {
							setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
							w.worm_t->enter();
							w.worm_howto->show();
							h.play();
						}
					}
					else if (key == 59 && pressed && i == 1 && j == 1) endGame();
				}
				return true;
			});
			
		}
	}

	startGame(title);
	return 0;
}