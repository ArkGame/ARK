#pragma once

#include <exception>
#include <string>
#include <mutex>

#include <librdkafka/rdkafkacpp.h>
#include "base/AFStringUtils.hpp"
#include "base/AFNoncopyable.hpp"

namespace ark {

using AFKafkaErrCode = RdKafka::ErrorCode;

namespace {

inline void setConfig(RdKafka::Conf* conf, const std::string& key, const std::string& value)
{
    std::string error;
    if (conf->set(key, value, error) == RdKafka::Conf::CONF_OK)
    {
        return;
    }
    throw std::runtime_error("Failed to set '" + key + "' to '" + value + "'! err=" + error);
}

inline void setConfig(RdKafka::Conf* conf, const std::string& key, RdKafka::EventCb* cbObj)
{
    std::string error;
    if (conf->set(key, cbObj, error) == RdKafka::Conf::CONF_OK)
    {
        return;
    }
    throw std::runtime_error("Failed to set '" + key + "'! err=" + error);
}

inline void setConfig(RdKafka::Conf* conf, const std::string& key, RdKafka::OffsetCommitCb* cbObj)
{
    std::string error;
    if (conf->set(key, cbObj, error) == RdKafka::Conf::CONF_OK)
    {
        return;
    }
    throw std::runtime_error("Failed to set '" + key + "'! err=" + error);
}

inline void setConfig(RdKafka::Conf* conf, const std::string& key, RdKafka::DeliveryReportCb* cbObj)
{
    std::string error;
    if (conf->set(key, cbObj, error) == RdKafka::Conf::CONF_OK)
    {
        return;
    }
    throw std::runtime_error("Failed to set '" + key + "'! err=" + error);
}

} // namespace

/**
 * @brief RdKafka::KafkaConsumer的简单封装
 */
class AFKafkaConsumer
{
public:
    using Message = RdKafka::Message;

public:
    /**
	 * @brief 构造函数
	 * @param brokers 逗号分隔的ip:port列表
	 * @param groupID
	 * @param topics 逗号分隔的topic列表
	 */
    AFKafkaConsumer(const std::string& brokers, const std::string& groupID, const std::string& topics,
        RdKafka::OffsetCommitCb* offsetCb = nullptr, RdKafka::EventCb* evcb = nullptr)
    {
        if (brokers.empty() || groupID.empty() || topics.empty())
        {
            throw std::runtime_error("Invalid parameters!");
        }

        auto conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        std::unique_ptr<RdKafka::Conf> confForAutoDel(conf); // for auto delete when throwing exception

        setConfig(conf, "metadata.broker.list", brokers);
        setConfig(conf, "group.id", groupID);
        setConfig(conf, "auto.offset.reset", "smallest");
        setConfig(conf, "enable.auto.commit", "false");
        setConfig(conf, "enable.auto.offset.store", "false");
        if (evcb)
        {
            setConfig(conf, "event_cb", evcb);
        }
        if (offsetCb)
        {
            setConfig(conf, "offset_commit_cb", offsetCb);
        }

        std::string error;
        std::unique_ptr<RdKafka::KafkaConsumer> consumer(RdKafka::KafkaConsumer::create(conf, error));
        if (!consumer)
        {
            throw std::runtime_error("Failed to create consumer! err=" + error);
        }

        std::vector<std::string> allTopics;
        AFStringUtils::split(allTopics, topics, ',');
        auto err = consumer->subscribe(allTopics);
        if (err)
        {
            throw std::runtime_error("Failed to subscribe to '" + topics + "'! err=" + RdKafka::err2str(err));
        }

        consumer_ = consumer.release();
    }

    ~AFKafkaConsumer()
    {
        //consumer_->close(); // 会卡死，先注释掉，后续看情况调整
        delete consumer_;
    }

    /**
	 * @brief 获取一条消息或者错误事件,触发回调函数
	 * @remark 使用delete释放消息内存
	 * @returns One of:
	 *  - proper message (Message::err() is ERR_NO_ERROR)
	 *  - error event (Message::err() is != ERR_NO_ERROR)
	 *  - timeout due to no message or event in timeoutMS
	 *    (RdKafka::Message::err() is ERR__TIMED_OUT)
	 */
    Message* Consume(int timeoutMS)
    {
        return consumer_->consume(timeoutMS);
    }

    /**
	 * @brief 异步提交Offset
	 */
    void AsyncCommit(Message* msg)
    {
        consumer_->commitAsync(msg);
    }

private:
    RdKafka::KafkaConsumer* consumer_;
};

/**
 * @brief RdKafka::Producer的简单封装
 */
class AFKafkaProducer
{
public:
    /**
	 * @brief 构造函数
	 * @param brokers 逗号分隔的ip:port列表
	 * @param topic
	 */
    AFKafkaProducer(const std::string& brokers, const std::string& topic,
        RdKafka::DeliveryReportCb* delivery_report_cb = nullptr, RdKafka::EventCb* event_cb = nullptr)
    {
        auto conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        std::unique_ptr<RdKafka::Conf> confForAutoDel(conf); // for auto delete when throwing exception

        setConfig(conf, "metadata.broker.list", brokers);
        if (event_cb != nullptr)
        {
            setConfig(conf, "event_cb", event_cb);
        }
        if (delivery_report_cb != nullptr)
        {
            setConfig(conf, "dr_cb", delivery_report_cb);
        }

        std::string error;
        auto tmpProducer = RdKafka::Producer::create(conf, error);
        if (!tmpProducer)
        {
            throw std::runtime_error("Failed to create producer! err=" + error);
        }
        std::unique_ptr<RdKafka::Producer> producer(tmpProducer); // for auto delete when throwing exception

        topic_ = RdKafka::Topic::create(tmpProducer, topic, nullptr, error);
        if (topic_ == nullptr)
        {
            throw std::runtime_error("Failed to create topic! topic=" + topic + " error=" + error);
        }

        producer_ = producer.release();
    }

    ~AFKafkaProducer()
    {
        producer_->flush(10000);
        delete topic_;
        delete producer_;
    }

    /**
	 * @brief 异步往kafka broker发送一条消息
	 * @param payload 发送给kafka broker的消息内容
	 * @param msgOpaque 调用送达通知回调函数dr_cb时，传递给该函数
	 * @return 表示成功或者失败的错误码，取值如下：
	 *  - ERR_NO_ERROR           - 成功写入队列
	 *  - ERR__QUEUE_FULL        - 队列满：queue.buffering.max.message
	 *  - ERR_MSG_SIZE_TOO_LARGE - 消息太大：messages.max.bytes
	 *  - ERR__UNKNOWN_PARTITION
	 *  - ERR__UNKNOWN_TOPIC
	 */
    AFKafkaErrCode Produce(const std::string& payload, void* msgOpaque = nullptr)
    {
        // 只有当送达通知回调函数返回之后，该消息才会被移出队列。
        // 所以produce返回队列满时，不能马上重试，只能在定期调用Poll()函数后再重试。
        return producer_->produce(topic_, RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY,
            const_cast<char*>(payload.data()), payload.size(), nullptr, msgOpaque);
    }

    /**
	* @brief 同步往kafka broker发送一条消息
	* @param payload 发送给kafka broker的消息内容
	* @return 表示成功或者失败的错误码
	*/
    AFKafkaErrCode SyncProduce(const std::string& payload)
    {
        AFKafkaErrCode err = static_cast<AFKafkaErrCode>(-12345);

        producer_->produce(topic_, RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY,
            const_cast<char*>(payload.data()), payload.size(), nullptr, &err);
        while (err == static_cast<AFKafkaErrCode>(-12345))
        {
            Poll(1000);
        }

        return err;
    }

    /**
	 * @brief 触发已完成事件的回调函数。程序必须定期调用该函数，以触发事件回调函数。
	 * @param timeoutMS 函数等待事件的时间（毫秒），0表示不等待，-1表示永久等待
	 *
	 * 事件包括：
	 *   - 送达通知，回调dr_cb
	 *   - 其他事件，调用event_cb
	 * @returns 返回事件个数
	 */
    int Poll(int timeoutMS = 0)
    {
        return producer_->poll(timeoutMS);
    }

private:
    RdKafka::Producer* producer_;
    RdKafka::Topic* topic_;
};

/**
* @brief KafkaProducer对象池，线程安全。
*/
class AFKafkaProducerPool : private AFNoncopyable
{
public:
    /**
	* @brief 构造Oracle操作对象
	* @throw occi::SQLException
	* @note 多线程环境下，使用者必须保证drcb和evcb是线程安全的
	*/
    AFKafkaProducerPool(size_t maxPooledProducers, const std::string& brokers, const std::string& topic,
        RdKafka::DeliveryReportCb* delivery_report_cb = nullptr, RdKafka::EventCb* event_cb = nullptr)
        : maxPooledProducers_(maxPooledProducers)
        , brokers_(brokers)
        , topic_(topic)
        , delivery_report_cb_(delivery_report_cb)
        , event_cb_(event_cb)
    {
    }

    ~AFKafkaProducerPool()
    {
        for (auto cli : pooledProducers_)
        {
            delete cli;
        }
    }

    /**
	* @brief 获取KafkaProducer对象，使用完毕后，要调用Put接口返还。
	*/
    AFKafkaProducer* Get()
    {
        lock_.lock();
        if (!pooledProducers_.empty())
        {
            auto cli = pooledProducers_.front();
            pooledProducers_.pop_front();
            lock_.unlock();
            return cli;
        }
        lock_.unlock();

        try
        {
            return new AFKafkaProducer(brokers_, topic_, delivery_report_cb_, event_cb_);
        }
        catch (...)
        {
        }

        return nullptr;
    }

    /**
	* @brief 返还通过Get接口获取的KafkaProducer对象。
	*/
    void Put(AFKafkaProducer* producer)
    {
        lock_.lock();
        if (pooledProducers_.size() < maxPooledProducers_)
        {
            pooledProducers_.emplace_back(producer);
            lock_.unlock();
        }
        else
        {
            lock_.unlock();
            delete producer;
        }
    }

private:
    const size_t maxPooledProducers_;
    const std::string brokers_;
    const std::string topic_;
    RdKafka::DeliveryReportCb* const delivery_report_cb_;
    RdKafka::EventCb* const event_cb_;

    std::mutex lock_;
    std::list<AFKafkaProducer*> pooledProducers_;
};

} // namespace ark