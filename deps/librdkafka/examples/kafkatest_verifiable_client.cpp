/*
 * Copyright (c) 2015, Confluent Inc
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * librdkafka version of the Java VerifiableProducer and VerifiableConsumer
 * for use with the official Kafka client tests.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <ctype.h>

#ifdef _MSC_VER
#include "../win32/wingetopt.h"
#elif _AIX
#include <unistd.h>
#else
#include <getopt.h>
#endif

/*
 * Typically include path in a real application would be
 * #include <librdkafka/rdkafkacpp.h>
 */
#include "rdkafkacpp.h"

static bool run = true;
static bool exit_eof = false;
static int verbosity = 1;

class Assignment {

public:
  static std::string name (const std::string &t, int partition) {
    std::stringstream stm;
    stm << t << "." << partition;
    return stm.str();
  }

  Assignment(): topic(""), partition(-1), consumedMessages(0),
                minOffset(-1), maxOffset(0) {
    printf("Created assignment\n");
  }
  Assignment(const Assignment &a) {
    topic = a.topic;
    partition = a.partition;
    consumedMessages = a.consumedMessages;
    minOffset = a.minOffset;
    maxOffset = a.maxOffset;
  }

  Assignment &operator=(const Assignment &a) {
    this->topic = a.topic;
    this->partition = a.partition;
    this->consumedMessages = a.consumedMessages;
    this->minOffset = a.minOffset;
    this->maxOffset = a.maxOffset;
    return *this;
  }

  int operator==(const Assignment &a) const {
    return !(this->topic == a.topic &&
             this->partition == a.partition);
  }

  int operator<(const Assignment &a) const {
    if (this->topic < a.topic) return 1;
    if (this->topic >= a.topic) return 0;
    return (this->partition < a.partition);
  }
  
  void setup (std::string t, int32_t p) {
    assert(!t.empty());
    assert(topic.empty() || topic == t);
    assert(partition == -1 || partition == p);
    topic = t;
    partition = p;
  }

  std::string topic;
  int partition;
  int consumedMessages;
  int64_t minOffset;
  int64_t maxOffset;
};




static struct {
  int maxMessages;

  struct {
    int numAcked;
    int numSent;
    int numErr;
  } producer;

  struct {
    int consumedMessages;
    int consumedMessagesLastReported;
    int consumedMessagesAtLastCommit;
    bool useAutoCommit;
    bool useAsyncCommit;
    std::map<std::string, Assignment> assignments;
  } consumer;
} state = {
  /* .maxMessages = */ -1
};


static RdKafka::KafkaConsumer *consumer;


static std::string now () {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t t = tv.tv_sec;
  struct tm *tm = localtime(&t);
  char buf[64];

  strftime(buf, sizeof(buf), "%H:%M:%S", tm);
  snprintf(buf+strlen(buf), sizeof(buf)-strlen(buf), ".%03d",
	   (int)(tv.tv_usec / 1000));

  return buf;
}


static time_t watchdog_last_kick;
static const int watchdog_timeout = 10;
static void sigwatchdog (int sig) {
  time_t t = time(NULL);
  if (watchdog_last_kick + watchdog_timeout <= t) {
    std::cerr << now() << ": WATCHDOG TIMEOUT (" <<
      (int)(t - watchdog_last_kick) << "s): TERMINATING" << std::endl;
    abort();
  }
}

static void watchdog_kick () {
  watchdog_last_kick = time(NULL);
  
  /* Safe guard against hangs-on-exit */
  alarm(watchdog_timeout);
}





static void errorString (const std::string &name,
			 const std::string &errmsg,
			 const std::string &topic,
			 const std::string *key,
			 const std::string &value) {
  std::cout << "{ "
	    << "\"name\": \"" << name << "\", "
    	    << "\"_time\": \"" << now() << "\", "
	    << "\"message\": \"" << errmsg << "\", "
	    << "\"topic\": \"" << topic << "\", "
	    << "\"key\": \"" << (key ? *key : "NULL") << "\", "
	    << "\"value\": \"" << value << "\" "
	    << "}" << std::endl;
}


static void successString (const std::string &name,
			   const std::string &topic,
			   int partition,
			   int64_t offset,
			   const std::string *key,
			   const std::string &value) {
  std::cout << "{ "
	    << "\"name\": \"" << name << "\", "
	    << "\"_time\": \"" << now() << "\", "
	    << "\"topic\": \"" << topic << "\", "
    	    << "\"partition\": " << partition << ", "
	    << "\"offset\": " << offset << ", "
	    << "\"key\": \"" << (key ? *key : "NULL") << "\", "
	    << "\"value\": \"" << value << "\" "
	    << "}" << std::endl;
}


#if FIXME
static void offsetStatus (bool success,
			  const std::string &topic,
			  int partition,
			  int64_t offset,
			  const std::string &errstr) {
  std::cout << "{ "
    "\"name\": \"offsets_committed\", " <<
    "\"success\": " << success << ", " <<
    "\"offsets\": [ " <<
    " { " <<
    " \"topic\": \"" << topic << "\", " <<
    " \"partition\": " << partition << ", " <<
    " \"offset\": " << (int)offset << ", " <<
    " \"error\": \"" << errstr << "\" " <<
    " } " <<
    "] }" << std::endl;
}
#endif


static void sigterm (int sig) {

  std::cerr << now() << ": Terminating" << std::endl;

  if (!run) {
    std::cerr << now() << ": Forced termination" << std::endl;
    exit(1);
  }
  run = false;
}


class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
 public:
  void dr_cb (RdKafka::Message &message) {
    if (message.err()) {
      state.producer.numErr++;
      errorString("producer_send_error", message.errstr(),
		  message.topic_name(),
		  message.key(),
		  std::string(static_cast<const char*>(message.payload()),
			      message.len()));
    } else {
      successString("producer_send_success",
		    message.topic_name(),
		    (int)message.partition(),
		    message.offset(),
		    message.key(),
		    std::string(static_cast<const char*>(message.payload()),
				message.len()));
      state.producer.numAcked++;
    }
  }
};


class ExampleEventCb : public RdKafka::EventCb {
 public:
  void event_cb (RdKafka::Event &event) {
    switch (event.type())
    {
      case RdKafka::Event::EVENT_ERROR:
        std::cerr << now() << ": ERROR (" << RdKafka::err2str(event.err()) << "): " <<
            event.str() << std::endl;
        if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
          run = false;
        break;

      case RdKafka::Event::EVENT_STATS:
        std::cerr << now() << ": \"STATS\": " << event.str() << std::endl;
        break;

      case RdKafka::Event::EVENT_LOG:
	std::cerr << now() << ": LOG-" << event.severity() << "-"
		  << event.fac() << ": " << event.str() << std::endl;
        break;

      default:
        std::cerr << now() << ": EVENT " << event.type() <<
            " (" << RdKafka::err2str(event.err()) << "): " <<
            event.str() << std::endl;
        break;
    }
  }
};


/* Use of this partitioner is pretty pointless since no key is provided
 * in the produce() call. */
class MyHashPartitionerCb : public RdKafka::PartitionerCb {
 public:
  int32_t partitioner_cb (const RdKafka::Topic *topic, const std::string *key,
                          int32_t partition_cnt, void *msg_opaque) {
    return djb_hash(key->c_str(), key->size()) % partition_cnt;
  }
 private:

  static inline unsigned int djb_hash (const char *str, size_t len) {
    unsigned int hash = 5381;
    for (size_t i = 0 ; i < len ; i++)
      hash = ((hash << 5) + hash) + str[i];
    return hash;
  }
};





/**
 * Print number of records consumed, every 100 messages or on timeout.
 */
static void report_records_consumed (int immediate) {
  std::map<std::string,Assignment> *assignments = &state.consumer.assignments;

  if (state.consumer.consumedMessages <=
      state.consumer.consumedMessagesLastReported + (immediate ? 0 : 100))
    return;

  std::cout << "{ "
      "\"name\": \"records_consumed\", " <<
      "\"count\": " << (state.consumer.consumedMessages -
                        state.consumer.consumedMessagesLastReported) << ", " <<
      "\"partitions\": [ ";

  for (std::map<std::string,Assignment>::iterator ii = assignments->begin() ;
       ii != assignments->end() ; ii++) {
    Assignment *a = &(*ii).second;
    assert(!a->topic.empty());
    std::cout << (ii == assignments->begin() ? "": ", ") << " { " <<
        " \"topic\": \"" << a->topic << "\", " <<
        " \"partition\": " << a->partition << ", " <<
        " \"minOffset\": " << a->minOffset << ", " <<
        " \"maxOffset\": " << a->maxOffset << " " <<
        " } ";
    a->minOffset = -1;
  }

  std::cout << "] }" << std::endl;

  state.consumer.consumedMessagesLastReported = state.consumer.consumedMessages;
}



/**
 * Commit every 1000 messages or whenever there is a consume timeout.
 */
static void do_commit (RdKafka::KafkaConsumer *consumer, int immediate) {
  if (!state.consumer.useAutoCommit &&
      (state.consumer.consumedMessagesAtLastCommit + (immediate ? 0 : 1000)) <
       state.consumer.consumedMessages) {

    /* Make sure we report consumption before commit,
     * otherwise tests may fail because of commit > consumed. */
    if (state.consumer.consumedMessagesLastReported <
        state.consumer.consumedMessages)
      report_records_consumed(1);

    std::cerr << now() << ": committing " <<
      (state.consumer.consumedMessages -
       state.consumer.consumedMessagesAtLastCommit) << " messages (" <<
      (state.consumer.useAsyncCommit ? "async":"sync") << ")" << std::endl;
    if (state.consumer.useAsyncCommit)
      consumer->commitAsync();
    else
      consumer->commitSync();

    state.consumer.consumedMessagesAtLastCommit =
      state.consumer.consumedMessages;
  }
}


void msg_consume(RdKafka::KafkaConsumer *consumer,
		 RdKafka::Message* msg, void* opaque) {
  switch (msg->err()) {
    case RdKafka::ERR__TIMED_OUT:
      /* Try reporting consumed messages */
      report_records_consumed(1);
      /* Commit one every consume() timeout instead of on every message.
       * Also commit on every 1000 messages, whichever comes first. */
      do_commit(consumer, 1);
      break;


    case RdKafka::ERR_NO_ERROR:
      {
        /* Real message */
	if (verbosity > 2)
	  std::cerr << now() << ": Read msg from " << msg->topic_name() <<
            " [" << (int)msg->partition() << "]  at offset " <<
            msg->offset() << std::endl;

        if (state.maxMessages >= 0 &&
            state.consumer.consumedMessages >= state.maxMessages)
          return;


        Assignment *a =
            &state.consumer.assignments[Assignment::name(msg->topic_name(),
                                                        msg->partition())];
        a->setup(msg->topic_name(), msg->partition());

        a->consumedMessages++;
        if (a->minOffset == -1)
          a->minOffset = msg->offset();
        if (a->maxOffset < msg->offset())
          a->maxOffset = msg->offset();

        if (msg->key()) {
	  if (verbosity >= 3)
	    std::cerr << now() << ": Key: " << *msg->key() << std::endl;
        }

	if (verbosity >= 3)
        fprintf(stderr, "%.*s\n",
                static_cast<int>(msg->len()),
                static_cast<const char *>(msg->payload()));

        state.consumer.consumedMessages++;

        report_records_consumed(0);

        do_commit(consumer, 0);
      }
      break;

    case RdKafka::ERR__PARTITION_EOF:
      /* Last message */
      if (exit_eof) {
        run = false;
      }
      break;

    case RdKafka::ERR__UNKNOWN_TOPIC:
    case RdKafka::ERR__UNKNOWN_PARTITION:
      std::cerr << now() << ": Consume failed: " << msg->errstr() << std::endl;
      run = false;
      break;

    case RdKafka::ERR_GROUP_COORDINATOR_NOT_AVAILABLE:
      std::cerr << now() << ": Warning: " << msg->errstr() << std::endl;
      break;

    default:
      /* Errors */
      std::cerr << now() << ": Consume failed: " << msg->errstr() << std::endl;
      run = false;
  }
}




class ExampleConsumeCb : public RdKafka::ConsumeCb {
 public:
  void consume_cb (RdKafka::Message &msg, void *opaque) {
    msg_consume(consumer_, &msg, opaque);
  }
  RdKafka::KafkaConsumer *consumer_;
};

class ExampleRebalanceCb : public RdKafka::RebalanceCb {
private:
  static void part_list_json (const std::vector<RdKafka::TopicPartition*> &partitions) {
    for (unsigned int i = 0 ; i < partitions.size() ; i++)
      std::cout << (i==0?"":", ") << "{ " <<
	" \"topic\": \"" << partitions[i]->topic() << "\", " <<
	" \"partition\": " << partitions[i]->partition() <<
	" }";
  }
 public:
  void rebalance_cb (RdKafka::KafkaConsumer *consumer,
		     RdKafka::ErrorCode err,
		     std::vector<RdKafka::TopicPartition*> &partitions) {
    std::cout << "{ " <<
      "\"name\": \"partitions_" << (err == RdKafka::ERR__ASSIGN_PARTITIONS ?
				    "assigned" : "revoked") << "\", " <<
      "\"partitions\": [ ";
    part_list_json(partitions);
    std::cout << "] }" << std::endl;

    if (err == RdKafka::ERR__ASSIGN_PARTITIONS)
      consumer->assign(partitions);
    else
      consumer->unassign();
  }
};



class ExampleOffsetCommitCb : public RdKafka::OffsetCommitCb {
 public:
  void offset_commit_cb (RdKafka::ErrorCode err,
                         std::vector<RdKafka::TopicPartition*> &offsets) {
    std::cerr << now() << ": Propagate offset for " << offsets.size() << " partitions, error: " << RdKafka::err2str(err) << std::endl;

    /* No offsets to commit, dont report anything. */
    if (err == RdKafka::ERR__NO_OFFSET)
      return;

    std::cout << "{ " <<
        "\"name\": \"offsets_committed\", " <<
        "\"success\": " << (err ? "false" : "true") << ", " <<
        "\"error\": \"" << (err ? RdKafka::err2str(err) : "") << "\", " <<
        "\"offsets\": [ ";
    assert(offsets.size() > 0);
    for (unsigned int i = 0 ; i < offsets.size() ; i++) {
      std::cout << (i == 0 ? "" : ", ") << "{ " <<
          " \"topic\": \"" << offsets[i]->topic() << "\", " <<
          " \"partition\": " << offsets[i]->partition() << ", " <<
          " \"offset\": " << (int)offsets[i]->offset() << ", " <<
          " \"error\": \"" <<
          (offsets[i]->err() ? RdKafka::err2str(offsets[i]->err()) : "") <<
          "\" " <<
          " }";
    }
    std::cout << " ] }" << std::endl;
  }
};



static void read_conf_file (const std::string &conf_file) {
  std::ifstream inf(conf_file.c_str());

  std::string line;
  while (std::getline(inf, line)) {
    std::cerr << now() << ": conf_file: " << conf_file << ": " << line << std::endl;
  }

  inf.close();
}




int main (int argc, char **argv) {
  std::string brokers = "localhost";
  std::string errstr;
  std::vector<std::string> topics;
  std::string conf_file;
  std::string mode = "P";
  int throughput = 0;
  int32_t partition = RdKafka::Topic::PARTITION_UA;
  bool do_conf_dump = false;
  MyHashPartitionerCb hash_partitioner;

  /*
   * Create configuration objects
   */
  RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

  {
    char hostname[128];
    gethostname(hostname, sizeof(hostname)-1);
    conf->set("client.id", std::string("rdkafka@") + hostname, errstr);
  }

  conf->set("debug", "cgrp,topic", errstr);

  for (int i = 1 ; i < argc ; i++) {
    const char *name = argv[i];
    const char *val = i+1 < argc ? argv[i+1] : NULL;

    if (val && !strncmp(val, "-", 1))
      val = NULL;

    std::cout << now() << ": argument: " << name << " " <<
        (val?val:"") << std::endl;

    if (val) {
      if (!strcmp(name, "--topic"))
	topics.push_back(val);
      else if (!strcmp(name, "--broker-list"))
	brokers = val;
      else if (!strcmp(name, "--max-messages"))
	state.maxMessages = atoi(val);
      else if (!strcmp(name, "--throughput"))
	throughput = atoi(val);
      else if (!strcmp(name, "--producer.config") ||
	       !strcmp(name, "--consumer.config"))
	read_conf_file(val);
      else if (!strcmp(name, "--group-id"))
	conf->set("group.id", val, errstr);
      else if (!strcmp(name, "--session-timeout"))
	conf->set("session.timeout.ms", val, errstr);
      else if (!strcmp(name, "--reset-policy")) {
	if (tconf->set("auto.offset.reset", val, errstr)) {
	  std::cerr << now() << ": " << errstr << std::endl;
	  exit(1);
	}
      } else if (!strcmp(name, "--assignment-strategy")) {
	/* The system tests pass the Java class name rather than
	 * the configuration value. Fix it.
	 * "org.apache.kafka.clients.consumer.RangeAssignor"
	 *  -> "Range" -> "range"
	 */
	char tmp[128];
	if (!strncmp(val, "org.apache", strlen("org.apache"))) {
	  const char *t = rindex(val, '.') + 1;
	  const char *t2 = strstr(t, "Assignor");
	  char *t3;
	  if (!t2)
	    t2 = t + strlen(t);
	  strncpy(tmp, t, (int)(t2-t));
	  tmp[(int)(t2-t)] = '\0';
	  for (t3 = tmp ; *t3 ; t3++)
	    *t3 = tolower(*t3);
	  std::cerr << now() << ": converted " << name << " "
		    << val << " to " << tmp << std::endl;
	  val = tmp;
	}

	if  (conf->set("partition.assignment.strategy", val, errstr)) {
	  std::cerr << now() << ": " << errstr << std::endl;
	  exit(1);
	}
      } else if (!strcmp(name, "--debug")) {
	conf->set("debug", val, errstr);
      } else {
	std::cerr << now() << ": Unknown option " << name << std::endl;
	exit(1);
      }

      i++;

    } else {
      if (!strcmp(name, "--consumer"))
	mode = "C";
      else if (!strcmp(name, "--producer"))
	mode = "P";
      else if (!strcmp(name, "--enable-autocommit")) {
	state.consumer.useAutoCommit = true;
	conf->set("enable.auto.commit", "true", errstr);
      } else if (!strcmp(name, "-v"))
	verbosity++;
      else if (!strcmp(name, "-q"))
	verbosity--;
      else {
	std::cerr << now() << ": Unknown option or missing argument to " << name << std::endl;
	exit(1);
      }
    }
  }

  if (topics.empty() || brokers.empty()) {
    std::cerr << now() << ": Missing --topic and --broker-list" << std::endl;
    exit(1);
  }


  /*
   * Set configuration properties
   */
  conf->set("metadata.broker.list", brokers, errstr);

  ExampleEventCb ex_event_cb;
  conf->set("event_cb", &ex_event_cb, errstr);

  if (do_conf_dump) {
    int pass;

    for (pass = 0 ; pass < 2 ; pass++) {
      std::list<std::string> *dump;
      if (pass == 0) {
        dump = conf->dump();
        std::cerr << now() << ": # Global config" << std::endl;
      } else {
        dump = tconf->dump();
        std::cerr << now() << ": # Topic config" << std::endl;
      }

      for (std::list<std::string>::iterator it = dump->begin();
           it != dump->end(); ) {
        std::cerr << *it << " = ";
        it++;
        std::cerr << *it << std::endl;
        it++;
      }
      std::cerr << std::endl;
    }
    exit(0);
  }

  signal(SIGINT, sigterm);
  signal(SIGTERM, sigterm);
  signal(SIGALRM,  sigwatchdog);


  if (mode == "P") {
    /*
     * Producer mode
     */

    ExampleDeliveryReportCb ex_dr_cb;

    /* Set delivery report callback */
    conf->set("dr_cb", &ex_dr_cb, errstr);

    /*
     * Create producer using accumulated global configuration.
     */
    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
      std::cerr << now() << ": Failed to create producer: " << errstr << std::endl;
      exit(1);
    }

    std::cerr << now() << ": % Created producer " << producer->name() << std::endl;

    /*
     * Create topic handle.
     */
    RdKafka::Topic *topic = RdKafka::Topic::create(producer, topics[0],
						   tconf, errstr);
    if (!topic) {
      std::cerr << now() << ": Failed to create topic: " << errstr << std::endl;
      exit(1);
    }

    static const int delay_us = throughput ? 1000000/throughput : 0;

    if (state.maxMessages == -1)
      state.maxMessages = 1000000; /* Avoid infinite produce */

    for (int i = 0 ; run && i < state.maxMessages ; i++) {
      /*
       * Produce message
       */
      std::ostringstream msg;
      msg << i;
      RdKafka::ErrorCode resp =
	producer->produce(topic, partition,
			  RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
			  const_cast<char *>(msg.str().c_str()),
			  msg.str().size(), NULL, NULL);
      if (resp != RdKafka::ERR_NO_ERROR) {
	errorString("producer_send_error",
		    RdKafka::err2str(resp), topic->name(), NULL, msg.str());
	state.producer.numErr++;
      } else {
	std::cerr << now() << ": % Produced message (" <<
	  msg.str().size() << " bytes)" << std::endl;
	state.producer.numSent++;
      }

      producer->poll(delay_us / 1000);
      watchdog_kick();
    }
    run = true;

    while (run && producer->outq_len() > 0) {
      std::cerr << now() << ": Waiting for " << producer->outq_len() << std::endl;
      producer->poll(50);
      watchdog_kick();
    }

    std::cerr << now() << ": " << state.producer.numAcked << "/" <<
      state.producer.numSent << "/" << state.maxMessages <<
      " msgs acked/sent/max, " << state.producer.numErr <<
      " errored" << std::endl;

    delete topic;
    delete producer;


  } else if (mode == "C") {
    /*
     * Consumer mode
     */

    tconf->set("auto.offset.reset", "smallest", errstr);

    /* Set default topic config */
    conf->set("default_topic_conf", tconf, errstr);

    ExampleRebalanceCb ex_rebalance_cb;
    conf->set("rebalance_cb", &ex_rebalance_cb, errstr);

    ExampleOffsetCommitCb ex_offset_commit_cb;
    conf->set("offset_commit_cb", &ex_offset_commit_cb, errstr);


    /*
     * Create consumer using accumulated global configuration.
     */
    consumer = RdKafka::KafkaConsumer::create(conf, errstr);
    if (!consumer) {
      std::cerr << now() << ": Failed to create consumer: " <<
	errstr << std::endl;
      exit(1);
    }

    std::cerr << now() << ": % Created consumer " << consumer->name() <<
      std::endl;

    /*
     * Subscribe to topic(s)
     */
    RdKafka::ErrorCode resp = consumer->subscribe(topics);
    if (resp != RdKafka::ERR_NO_ERROR) {
      std::cerr << now() << ": Failed to subscribe to " << topics.size() << " topics: "
		<< RdKafka::err2str(resp) << std::endl;
      exit(1);
    }

    /*
     * Consume messages
     */
    while (run) {
      RdKafka::Message *msg = consumer->consume(500);
      msg_consume(consumer, msg, NULL);
      delete msg;
      watchdog_kick();
    }

    /* Final commit */
    do_commit(consumer, 1);

    /*
     * Stop consumer
     */
    consumer->close();

    delete consumer;
  }


  /*
   * Wait for RdKafka to decommission.
   * This is not strictly needed (when check outq_len() above), but
   * allows RdKafka to clean up all its resources before the application
   * exits so that memory profilers such as valgrind wont complain about
   * memory leaks.
   */
  RdKafka::wait_destroyed(5000);

  std::cerr << now() << ": EXITING WITH RETURN VALUE 0" << std::endl;
  return 0;
}
