#ifndef SOURCEMODEL__MATH_FILTERS_AAF_H
#define SOURCEMODEL__MATH_FILTERS_AAF_H

#include <array>

inline constexpr int                              aafFilterLen = 503;
inline constexpr std::array<double, aafFilterLen> aafFilterCoefs = {
    -0.000000212869489572, 0.000000000000000000,  0.000000270452054199,
    -0.000000602800804358, 0.000001001120963207,  -0.000001469185539219,
    0.000002010401679117,  -0.000002627745088366, 0.000003323693517880,
    -0.000004100159841464, 0.000004958425282390,  -0.000005899073378598,
    0.000006921925303828,  -0.000008025977186337, 0.000009209340087220,
    -0.000010469183316510, 0.000011801681776713,  -0.000013201968030210,
    0.000014664089788299,  -0.000016180973515857, 0.000017744394835931,
    -0.000019344956403191, 0.000020972073893751,  -0.000022613970731388,
    0.000024257682136408,  -0.000025889069043569, 0.000027492842389283,
    -0.000029052598216054, 0.000030550863983806,  -0.000031969156413519,
    0.000033288051118737,  -0.000034487265120612, 0.000035545748689565,
    -0.000036441792889239, 0.000037153145698420,  -0.000037657142541343,
    0.000037930847374144,  -0.000037951204989105, 0.000037695204063361,
    -0.000037140050377192, 0.000036263349524658,  -0.000035043298335949,
    0.000033458884128074,  -0.000031490090798822, 0.000029118110679365,
    -0.000026325560964112, 0.000023096703443445,  -0.000019417666176592,
    0.000015276665658667,  -0.000010664227959381, 0.000005573407240839,
    -0.000000000000000006, -0.000006057246672053, 0.000012596434566102,
    -0.000019612323352207, 0.000027096150907907,  -0.000035035466799618,
    0.000043413980738847,  -0.000052211427824385, 0.000061403452355923,
    -0.000070961511967725, 0.000080852803779614,  -0.000091040214198540,
    0.000101482293925095,  -0.000112133259628715, 0.000122943023648790,
    -0.000133857252960868, 0.000144817458514750,  -0.000155761115906523,
    0.000166621818189160,  -0.000177329461457516, 0.000187810463662498,
    -0.000197988016919258, 0.000207782373372825,  -0.000217111164475961,
    0.000225889753316725,  -0.000234031619409768, 0.000241448775136146,
    -0.000248052212783343, 0.000253752380900875,  -0.000258459688449275,
    0.000262085034982493,  -0.000264540364867498, 0.000265739243311483,
    -0.000265597451738337, 0.000264033599833399,  -0.000260969751360276,
    0.000256332060648268,  -0.000250051416453990, 0.000242064089718327,
    -0.000232312381571956, 0.000220745267790299,  -0.000207319035762347,
    0.000191997909921413,  -0.000174754661488726, 0.000155571198303171,
    -0.000134439130458503, 0.000111360307436515,  -0.000086347322419467,
    0.000059423979483883,  -0.000030625719420751, -0.000000000000000003,
    0.000032393373406439,  -0.000066481970803597, 0.000102180517962285,
    -0.000139390737846849, 0.000178001256504065,  -0.000217887576601960,
    0.000258912121548535,  -0.000300924352834237, 0.000343760962933496,
    -0.000387246145766426, 0.000431191946367174,  -0.000475398691026682,
    0.000519655498783844,  -0.000563740874722944, 0.000607423385105216,
    -0.000650462413919650, 0.000692608999980455,  -0.000733606753232948,
    0.000773192848458052,  -0.000811099094085143, 0.000847053073345218,
    -0.000880779354514324, 0.000912000766521773,  -0.000940439735725522,
    0.000965819679195133,  -0.000987866449391316, 0.001006309824694053,
    -0.001020885039811427, 0.001031334349701115,  -0.001037408620258655,
    0.001038868938674456,  -0.001035488236036701, 0.001027052914462650,
    -0.001013364470779922, 0.000994241108551806,  -0.000969519330051810,
    0.000939055499641052,  -0.000902727369893797, 0.000860435561747725,
    -0.000812104989933270, 0.000757686224956545,  -0.000697156782979882,
    0.000630522335056971,  -0.000557817827340849, 0.000479108504094183,
    -0.000394490825585515, 0.000304093273260374,  -0.000208077034926104,
    0.000106636563086308,  -0.000000000000000058, -0.000111570536471122,
    0.000227778811642190,  -0.000348294482254800, 0.000472753258439622,
    -0.000600757222993758, 0.000731875310899778,  -0.000865643951312833,
    0.001001567873512206,  -0.001139121077556855, 0.001277747969613853,
    -0.001416864661134715, 0.001555860430253329,  -0.001694099342962529,
    0.001830922030810986,  -0.001965647621042640, 0.002097575814282437,
    -0.002225989104066834, 0.002350155131718322,  -0.002469329169284828,
    0.002582756722501843,  -0.002689676245001806, 0.002789321954287755,
    -0.002880926739313089, 0.002963725148873845,  -0.003036956449422490,
    0.003099867740360722,  -0.003151717114364735, 0.003191776849843953,
    -0.003219336622236219, 0.003233706720501224,  -0.003234221254893793,
    0.003220241341879888,  -0.003191158251903003, 0.003146396505622822,
    -0.003085416904224956, 0.003007719479450608,  -0.002912846349110917,
    0.002800384464039457,  -0.002669968232691877, 0.002521282009930611,
    -0.002354062436924272, 0.002168100619559804,  -0.001963244133289157,
    0.001739398842933134,  -0.001496530526615062, 0.001234666293721095,
    -0.000953895787551730, 0.000654372164161724,  -0.000336312839764206,
    0.000000000000000092,  0.000354219134658051,  -0.000725932292086259,
    0.001114662383954068,  -0.001519867996041976, 0.001940944134942160,
    -0.002377223231535966, 0.002827976400517917,  -0.003292414954087729,
    0.003769692166800473,  -0.004258905287414259, 0.004759097792452255,
    -0.005269261875070781, 0.005788341161726039,  -0.006315233648059669,
    0.006848794844373450,  -0.007387841120055555, 0.007931153235347571,
    -0.008477480047920856, 0.009025542380849272,  -0.009574037037752236,
    0.010121640950114853,  -0.010667015441097683, 0.011208810589515917,
    -0.011745669677107726, 0.012276233701724783,  -0.012799145938666719,
    0.013313056532050454,  -0.013816627097853943, 0.014308535320105930,
    -0.014787479521608483, 0.015252183190580153,  -0.015701399444690602,
    0.016133915414131453,  -0.016548556525618808, 0.016944190669564256,
    -0.017319732233071088, 0.017674145981914257,  -0.018006450775242999,
    0.018315723097401510,  -0.018601100391992573, 0.018861784184110195,
    -0.019097042977528383, 0.019306214914570616,  -0.019488710187360328,
    0.019644013190200663,  -0.019771684403917553, 0.019871362004128597,
    -0.019942763186579567, 0.019985685203882660,  0.980000299352128512,
    0.019985685203882660,  -0.019942763186579567, 0.019871362004128597,
    -0.019771684403917553, 0.019644013190200663,  -0.019488710187360328,
    0.019306214914570616,  -0.019097042977528383, 0.018861784184110195,
    -0.018601100391992573, 0.018315723097401510,  -0.018006450775242999,
    0.017674145981914257,  -0.017319732233071088, 0.016944190669564256,
    -0.016548556525618808, 0.016133915414131453,  -0.015701399444690602,
    0.015252183190580153,  -0.014787479521608483, 0.014308535320105930,
    -0.013816627097853943, 0.013313056532050454,  -0.012799145938666719,
    0.012276233701724783,  -0.011745669677107726, 0.011208810589515917,
    -0.010667015441097683, 0.010121640950114853,  -0.009574037037752236,
    0.009025542380849272,  -0.008477480047920856, 0.007931153235347571,
    -0.007387841120055555, 0.006848794844373450,  -0.006315233648059669,
    0.005788341161726039,  -0.005269261875070781, 0.004759097792452255,
    -0.004258905287414259, 0.003769692166800473,  -0.003292414954087729,
    0.002827976400517917,  -0.002377223231535966, 0.001940944134942158,
    -0.001519867996041976, 0.001114662383954068,  -0.000725932292086259,
    0.000354219134658051,  0.000000000000000092,  -0.000336312839764206,
    0.000654372164161724,  -0.000953895787551730, 0.001234666293721095,
    -0.001496530526615062, 0.001739398842933134,  -0.001963244133289157,
    0.002168100619559804,  -0.002354062436924272, 0.002521282009930611,
    -0.002669968232691877, 0.002800384464039457,  -0.002912846349110917,
    0.003007719479450608,  -0.003085416904224956, 0.003146396505622822,
    -0.003191158251903003, 0.003220241341879888,  -0.003234221254893793,
    0.003233706720501224,  -0.003219336622236219, 0.003191776849843953,
    -0.003151717114364735, 0.003099867740360722,  -0.003036956449422490,
    0.002963725148873845,  -0.002880926739313089, 0.002789321954287755,
    -0.002689676245001806, 0.002582756722501843,  -0.002469329169284828,
    0.002350155131718322,  -0.002225989104066834, 0.002097575814282437,
    -0.001965647621042640, 0.001830922030810988,  -0.001694099342962529,
    0.001555860430253329,  -0.001416864661134715, 0.001277747969613853,
    -0.001139121077556855, 0.001001567873512207,  -0.000865643951312833,
    0.000731875310899778,  -0.000600757222993758, 0.000472753258439622,
    -0.000348294482254799, 0.000227778811642190,  -0.000111570536471122,
    -0.000000000000000058, 0.000106636563086308,  -0.000208077034926104,
    0.000304093273260374,  -0.000394490825585515, 0.000479108504094183,
    -0.000557817827340849, 0.000630522335056971,  -0.000697156782979882,
    0.000757686224956545,  -0.000812104989933270, 0.000860435561747725,
    -0.000902727369893797, 0.000939055499641052,  -0.000969519330051810,
    0.000994241108551806,  -0.001013364470779922, 0.001027052914462650,
    -0.001035488236036701, 0.001038868938674456,  -0.001037408620258655,
    0.001031334349701115,  -0.001020885039811427, 0.001006309824694053,
    -0.000987866449391316, 0.000965819679195133,  -0.000940439735725522,
    0.000912000766521773,  -0.000880779354514324, 0.000847053073345218,
    -0.000811099094085143, 0.000773192848458052,  -0.000733606753232948,
    0.000692608999980455,  -0.000650462413919650, 0.000607423385105216,
    -0.000563740874722944, 0.000519655498783844,  -0.000475398691026682,
    0.000431191946367174,  -0.000387246145766426, 0.000343760962933496,
    -0.000300924352834237, 0.000258912121548535,  -0.000217887576601960,
    0.000178001256504065,  -0.000139390737846849, 0.000102180517962285,
    -0.000066481970803597, 0.000032393373406439,  -0.000000000000000003,
    -0.000030625719420751, 0.000059423979483883,  -0.000086347322419467,
    0.000111360307436516,  -0.000134439130458503, 0.000155571198303171,
    -0.000174754661488726, 0.000191997909921413,  -0.000207319035762347,
    0.000220745267790299,  -0.000232312381571956, 0.000242064089718326,
    -0.000250051416453990, 0.000256332060648268,  -0.000260969751360276,
    0.000264033599833399,  -0.000265597451738337, 0.000265739243311483,
    -0.000264540364867498, 0.000262085034982493,  -0.000258459688449275,
    0.000253752380900875,  -0.000248052212783343, 0.000241448775136146,
    -0.000234031619409768, 0.000225889753316725,  -0.000217111164475961,
    0.000207782373372825,  -0.000197988016919258, 0.000187810463662498,
    -0.000177329461457516, 0.000166621818189160,  -0.000155761115906523,
    0.000144817458514750,  -0.000133857252960868, 0.000122943023648790,
    -0.000112133259628715, 0.000101482293925095,  -0.000091040214198540,
    0.000080852803779614,  -0.000070961511967725, 0.000061403452355923,
    -0.000052211427824385, 0.000043413980738847,  -0.000035035466799618,
    0.000027096150907907,  -0.000019612323352207, 0.000012596434566102,
    -0.000006057246672053, -0.000000000000000006, 0.000005573407240839,
    -0.000010664227959381, 0.000015276665658667,  -0.000019417666176592,
    0.000023096703443445,  -0.000026325560964112, 0.000029118110679365,
    -0.000031490090798822, 0.000033458884128074,  -0.000035043298335949,
    0.000036263349524658,  -0.000037140050377192, 0.000037695204063361,
    -0.000037951204989105, 0.000037930847374144,  -0.000037657142541343,
    0.000037153145698420,  -0.000036441792889239, 0.000035545748689565,
    -0.000034487265120612, 0.000033288051118737,  -0.000031969156413519,
    0.000030550863983806,  -0.000029052598216054, 0.000027492842389283,
    -0.000025889069043569, 0.000024257682136408,  -0.000022613970731388,
    0.000020972073893751,  -0.000019344956403191, 0.000017744394835931,
    -0.000016180973515857, 0.000014664089788299,  -0.000013201968030210,
    0.000011801681776713,  -0.000010469183316510, 0.000009209340087220,
    -0.000008025977186337, 0.000006921925303828,  -0.000005899073378598,
    0.000004958425282390,  -0.000004100159841464, 0.000003323693517880,
    -0.000002627745088366, 0.000002010401679117,  -0.000001469185539219,
    0.000001001120963207,  -0.000000602800804358, 0.000000270452054199,
    0.000000000000000000,  -0.000000212869489572,
};

inline const std::vector<std::array<double, 6>> aafFilterCoefsIIR = {
    {1, 1, 2, 1, 0.8460843609236184, 1.8332068676890185},
};

#endif  // SOURCEMODEL__MATH_FILTERS_AAF_H