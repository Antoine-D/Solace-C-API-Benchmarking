###Solace Stress Test C###

####Run It####
To run run the bash script `run.sh` from SolaceStressTests/C. `sh run.sh`
By default it will pin cores 22 and 23 for the consumer and cores 24 and 25 for the producer (change taskset params. The output from the consumer is directed to `consumer.out` while the output from the producer is directed to `producer.out`.

Beforehand you will need to alter the config.json file to your liking (all fields are required).
```
{
  "hostname": "<hostname or ip>:<port (if not port provided then defaults to >",
  "vpn": "<vpn name>",
  "username": "<username>",
  "password": "<password>",
  "topic": "<topic name>",
  "warmups": <number of warmups to perform before actual recording>,
  "runs": <number of runs>,
  "threshold": <threshold for the number of microseconds between sends/receives that will cause a print of that time between sends/receives>,
  "payload": <size of payload in bytes>,
  "direct": <whether or not a direct send (boolean)>
}
```

Ex: `./SolaceConsumer config.json` and `./SolaceProducer config.json`


####Example Output####
Using config:
```
{
  "hostname": "_____________",
  "vpn": "_____________",
  "username": "_____________",
  "password": "_____________",
  "topic": "some/random/topic",
  "warmups": 1000000,
  "runs": 2000000,
  "threshold": 1000,
  "payload": 8,
  "direct": true
}
```

```
-bash-4.1$ ./SolaceProducer config.json

Session properties:
Setting SESSION_HOST to: _____________
Setting SESSION_VPN_NAME to: _____________
Setting SESSION_USERNAME to: _____________
Setting SESSION_PASSWORD to: _____________
Connected.
About to send 2000000 message(s) of 8 bytes to topic 'some/random/topic'...

--- Producer ---
 Times between sends:


 Stats: 0-3us: 1157192   3-6us: 839492   6-9us: 2788     9-20us: 501     20us+: 26       1000us+: 0
```

```
-bash-4.1$ ./SolaceConsumer config.json

Session properties:
Setting SESSION_HOST to: _____________
Setting SESSION_VPN_NAME to: _____________
Setting SESSION_USERNAME to: _____________
Setting SESSION_PASSWORD to: _____________
Connected.
warmup count: 1000000    run count: 2000000     Waiting for 2000000 message(s)......
Exiting.

--- Consumer ---
 Times between receives:
Time b/t msg found over 500 us: 853 us
Time b/t msg found over 500 us: 921 us
Time b/t msg found over 500 us: 1016 us
Time b/t msg found over 500 us: 983 us
Time b/t msg found over 500 us: 1015 us
Time b/t msg found over 500 us: 1004 us
Time b/t msg found over 500 us: 1037 us
Time b/t msg found over 500 us: 915 us
Time b/t msg found over 500 us: 957 us
Time b/t msg found over 500 us: 991 us
Time b/t msg found over 500 us: 974 us
Time b/t msg found over 500 us: 1000 us
Time b/t msg found over 500 us: 995 us
Time b/t msg found over 500 us: 1001 us


 Stats: 0-3us: 1856210   3-6us: 941      6-9us: 147      9-20us: 2688    20us+: 140013   1000us+: 14
```
