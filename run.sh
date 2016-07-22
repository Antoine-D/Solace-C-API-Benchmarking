cd include/jsmn/;
make;
cd ../.. ;
make;
export LD_LIBRARY_PATH=lib;
(
  ( ./SolaceConsumer config.json > consumer.out & );
  sleep 5 ;
  ( ./SolaceProducer config.json > producer.out & );
)
