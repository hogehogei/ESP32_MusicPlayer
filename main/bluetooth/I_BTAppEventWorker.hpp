
#ifndef     I_BT_APP_EVENT_WORKER_HPP_INCLUDED
#define     I_BT_APP_EVENT_WORKER_HPP_INCLUDED

/**
 * @brief   Bluetooth のイベントに応じた処理を別タスクで実行するための、インターフェースクラス
 *          何らかのイベントを受け取った/送信した時、実行したい処理を
 *          このクラスを継承したクラスの Invoke メソッドに記述しておく。
 *          イベントを受け取ったとき、ワーカーキューに対応する処理を記載したクラスがキューイングされ
 *          別タスクでイベントに対応した処理が順次実行される。
 **/
class I_BTAppEventWorker
{
public:

    virtual ~I_BTAppEventWorker() {}

    /**
     * @brief    イベントに応じたワーカーを起動する。
     * @return   true なら成功、false なら失敗
     **/
    virtual bool Invoke() = 0;
};

#endif    // I_BT_APP_EVENT_WORKER_HPP_INCLUDED
