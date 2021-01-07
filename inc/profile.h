#ifndef PROFILE_H
#define PROFILE_H 1
/* $Id: profile.h,v 1.2 1999/01/03 02:06:21 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/* DEFINE PROFILE if you want to turn on profiling */




/************************************************************************/
/*									*/
/*	(C) Copyright 1989-92 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/


#ifdef PROFILE
#define ASM(OPT,OPT2) asm(OPT2); asm(OPT)
#else
#ifdef ISC
  /* ISC 386 version needs assembler labels to make the dispatch table */
#define ASM(OPT,OPT2) asm(OPT)
#else
#define ASM(OPT,OPT2)
#endif /* ISC */

#endif /* PROFILE */



#define CASE000 ASM("_op000:", "	.globl _op000"); case000
#define CASE001 ASM("_op001:", "	.globl _op001"); case001
#define CASE002 ASM("_op002:", "	.globl _op002"); case002
#define CASE003 ASM("_op003:", "	.globl _op003"); case003
#define CASE004 ASM("_op004:", "	.globl _op004"); case004
#define CASE005 ASM("_op005:", "	.globl _op005"); case005
#define CASE006 ASM("_op006:", "	.globl _op006"); case006
#define CASE007 ASM("_op007:", "	.globl _op007"); case007
#define CASE010 ASM("_op010:", "	.globl _op010"); case010
#define CASE011 ASM("_op011:", "	.globl _op011"); case011
#define CASE012 ASM("_op012:", "	.globl _op012"); case012
#define CASE013 ASM("_op013:", "	.globl _op013"); case013
#define CASE014 ASM("_op014:", "	.globl _op014"); case014
#define CASE015 ASM("_op015:", "	.globl _op015"); case015
#define CASE016 ASM("_op016:", "	.globl _op016"); case016
#define CASE017 ASM("_op017:", "	.globl _op017"); case017
#define CASE020 ASM("_op020:", "	.globl _op020"); case020
#define CASE021 ASM("_op021:", "	.globl _op021"); case021
#define CASE022 ASM("_op022:", "	.globl _op022"); case022
#define CASE023 ASM("_op023:", "	.globl _op023"); case023
#define CASE024 ASM("_op024:", "	.globl _op024"); case024
#define CASE025 ASM("_op025:", "	.globl _op025"); case025
#define CASE026 ASM("_op026:", "	.globl _op026"); case026
#define CASE027 ASM("_op027:", "	.globl _op027"); case027
#define CASE030 ASM("_op030:", "	.globl _op030"); case030
#define CASE031 ASM("_op031:", "	.globl _op031"); case031
#define CASE032 ASM("_op032:", "	.globl _op032"); case032
#define CASE033 ASM("_op033:", "	.globl _op033"); case033
#define CASE034 ASM("_op034:", "	.globl _op034"); case034
#define CASE035 ASM("_op035:", "	.globl _op035"); case035
#define CASE036 ASM("_op036:", "	.globl _op036"); case036
#define CASE037 ASM("_op037:", "	.globl _op037"); case037
#define CASE040 ASM("_op040:", "	.globl _op040"); case040
#define CASE041 ASM("_op041:", "	.globl _op041"); case041
#define CASE042 ASM("_op042:", "	.globl _op042"); case042
#define CASE043 ASM("_op043:", "	.globl _op043"); case043
#define CASE044 ASM("_op044:", "	.globl _op044"); case044
#define CASE045 ASM("_op045:", "	.globl _op045"); case045
#define CASE046 ASM("_op046:", "	.globl _op046"); case046
#define CASE047 ASM("_op047:", "	.globl _op047"); case047
#define CASE050 ASM("_op050:", "	.globl _op050"); case050
#define CASE051 ASM("_op051:", "	.globl _op051"); case051
#define CASE052 ASM("_op052:", "	.globl _op052"); case052
#define CASE053 ASM("_op053:", "	.globl _op053"); case053
#define CASE054 ASM("_op054:", "	.globl _op054"); case054
#define CASE055 ASM("_op055:", "	.globl _op055"); case055
#define CASE056 ASM("_op056:", "	.globl _op056"); case056
#define CASE057 ASM("_op057:", "	.globl _op057"); case057
#define CASE060 ASM("_op060:", "	.globl _op060"); case060
#define CASE061 ASM("_op061:", "	.globl _op061"); case061
#define CASE062 ASM("_op062:", "	.globl _op062"); case062
#define CASE063 ASM("_op063:", "	.globl _op063"); case063
#define CASE064 ASM("_op064:", "	.globl _op064"); case064
#define CASE065 ASM("_op065:", "	.globl _op065"); case065
#define CASE066 ASM("_op066:", "	.globl _op066"); case066
#define CASE067 ASM("_op067:", "	.globl _op067"); case067
#define CASE070 ASM("_op070:", "	.globl _op070"); case070
#define CASE071 ASM("_op071:", "	.globl _op071"); case071
#define CASE072 ASM("_op072:", "	.globl _op072"); case072
#define CASE073 ASM("_op073:", "	.globl _op073"); case073
#define CASE074 ASM("_op074:", "	.globl _op074"); case074
#define CASE075 ASM("_op075:", "	.globl _op075"); case075
#define CASE076 ASM("_op076:", "	.globl _op076"); case076
#define CASE077 ASM("_op077:", "	.globl _op077"); case077
#define CASE100 ASM("_op100:", "	.globl _op100"); case100
#define CASE101 ASM("_op101:", "	.globl _op101"); case101
#define CASE102 ASM("_op102:", "	.globl _op102"); case102
#define CASE103 ASM("_op103:", "	.globl _op103"); case103
#define CASE104 ASM("_op104:", "	.globl _op104"); case104
#define CASE105 ASM("_op105:", "	.globl _op105"); case105
#define CASE106 ASM("_op106:", "	.globl _op106"); case106
#define CASE107 ASM("_op107:", "	.globl _op107"); case107
#define CASE110 ASM("_op110:", "	.globl _op110"); case110
#define CASE111 ASM("_op111:", "	.globl _op111"); case111
#define CASE112 ASM("_op112:", "	.globl _op112"); case112
#define CASE113 ASM("_op113:", "	.globl _op113"); case113
#define CASE114 ASM("_op114:", "	.globl _op114"); case114
#define CASE115 ASM("_op115:", "	.globl _op115"); case115
#define CASE116 ASM("_op116:", "	.globl _op116"); case116
#define CASE117 ASM("_op117:", "	.globl _op117"); case117
#define CASE120 ASM("_op120:", "	.globl _op120"); case120
#define CASE121 ASM("_op121:", "	.globl _op121"); case121
#define CASE122 ASM("_op122:", "	.globl _op122"); case122
#define CASE123 ASM("_op123:", "	.globl _op123"); case123
#define CASE124 ASM("_op124:", "	.globl _op124"); case124
#define CASE125 ASM("_op125:", "	.globl _op125"); case125
#define CASE126 ASM("_op126:", "	.globl _op126"); case126
#define CASE127 ASM("_op127:", "	.globl _op127"); case127
#define CASE130 ASM("_op130:", "	.globl _op130"); case130
#define CASE131 ASM("_op131:", "	.globl _op131"); case131
#define CASE132 ASM("_op132:", "	.globl _op132"); case132
#define CASE133 ASM("_op133:", "	.globl _op133"); case133
#define CASE134 ASM("_op134:", "	.globl _op134"); case134
#define CASE135 ASM("_op135:", "	.globl _op135"); case135
#define CASE136 ASM("_op136:", "	.globl _op136"); case136
#define CASE137 ASM("_op137:", "	.globl _op137"); case137
#define CASE140 ASM("_op140:", "	.globl _op140"); case140
#define CASE141 ASM("_op141:", "	.globl _op141"); case141
#define CASE142 ASM("_op142:", "	.globl _op142"); case142
#define CASE143 ASM("_op143:", "	.globl _op143"); case143
#define CASE144 ASM("_op144:", "	.globl _op144"); case144
#define CASE145 ASM("_op145:", "	.globl _op145"); case145
#define CASE146 ASM("_op146:", "	.globl _op146"); case146
#define CASE147 ASM("_op147:", "	.globl _op147"); case147
#define CASE150 ASM("_op150:", "	.globl _op150"); case150
#define CASE151 ASM("_op151:", "	.globl _op151"); case151
#define CASE152 ASM("_op152:", "	.globl _op152"); case152
#define CASE153 ASM("_op153:", "	.globl _op153"); case153
#define CASE154 ASM("_op154:", "	.globl _op154"); case154
#define CASE155 ASM("_op155:", "	.globl _op155"); case155
#define CASE156 ASM("_op156:", "	.globl _op156"); case156
#define CASE157 ASM("_op157:", "	.globl _op157"); case157
#define CASE160 ASM("_op160:", "	.globl _op160"); case160
#define CASE161 ASM("_op161:", "	.globl _op161"); case161
#define CASE162 ASM("_op162:", "	.globl _op162"); case162
#define CASE163 ASM("_op163:", "	.globl _op163"); case163
#define CASE164 ASM("_op164:", "	.globl _op164"); case164
#define CASE165 ASM("_op165:", "	.globl _op165"); case165
#define CASE166 ASM("_op166:", "	.globl _op166"); case166
#define CASE167 ASM("_op167:", "	.globl _op167"); case167
#define CASE170 ASM("_op170:", "	.globl _op170"); case170
#define CASE171 ASM("_op171:", "	.globl _op171"); case171
#define CASE172 ASM("_op172:", "	.globl _op172"); case172
#define CASE173 ASM("_op173:", "	.globl _op173"); case173
#define CASE174 ASM("_op174:", "	.globl _op174"); case174
#define CASE175 ASM("_op175:", "	.globl _op175"); case175
#define CASE176 ASM("_op176:", "	.globl _op176"); case176
#define CASE177 ASM("_op177:", "	.globl _op177"); case177
#define CASE200 ASM("_op200:", "	.globl _op200"); case200
#define CASE201 ASM("_op201:", "	.globl _op201"); case201
#define CASE202 ASM("_op202:", "	.globl _op202"); case202
#define CASE203 ASM("_op203:", "	.globl _op203"); case203
#define CASE204 ASM("_op204:", "	.globl _op204"); case204
#define CASE205 ASM("_op205:", "	.globl _op205"); case205
#define CASE206 ASM("_op206:", "	.globl _op206"); case206
#define CASE207 ASM("_op207:", "	.globl _op207"); case207
#define CASE210 ASM("_op210:", "	.globl _op210"); case210
#define CASE211 ASM("_op211:", "	.globl _op211"); case211
#define CASE212 ASM("_op212:", "	.globl _op212"); case212
#define CASE213 ASM("_op213:", "	.globl _op213"); case213
#define CASE214 ASM("_op214:", "	.globl _op214"); case214
#define CASE215 ASM("_op215:", "	.globl _op215"); case215
#define CASE216 ASM("_op216:", "	.globl _op216"); case216
#define CASE217 ASM("_op217:", "	.globl _op217"); case217
#define CASE220 ASM("_op220:", "	.globl _op220"); case220
#define CASE221 ASM("_op221:", "	.globl _op221"); case221
#define CASE222 ASM("_op222:", "	.globl _op222"); case222
#define CASE223 ASM("_op223:", "	.globl _op223"); case223
#define CASE224 ASM("_op224:", "	.globl _op224"); case224
#define CASE225 ASM("_op225:", "	.globl _op225"); case225
#define CASE226 ASM("_op226:", "	.globl _op226"); case226
#define CASE227 ASM("_op227:", "	.globl _op227"); case227
#define CASE230 ASM("_op230:", "	.globl _op230"); case230
#define CASE231 ASM("_op231:", "	.globl _op231"); case231
#define CASE232 ASM("_op232:", "	.globl _op232"); case232
#define CASE233 ASM("_op233:", "	.globl _op233"); case233
#define CASE234 ASM("_op234:", "	.globl _op234"); case234
#define CASE235 ASM("_op235:", "	.globl _op235"); case235
#define CASE236 ASM("_op236:", "	.globl _op236"); case236
#define CASE237 ASM("_op237:", "	.globl _op237"); case237
#define CASE240 ASM("_op240:", "	.globl _op240"); case240
#define CASE241 ASM("_op241:", "	.globl _op241"); case241
#define CASE242 ASM("_op242:", "	.globl _op242"); case242
#define CASE243 ASM("_op243:", "	.globl _op243"); case243
#define CASE244 ASM("_op244:", "	.globl _op244"); case244
#define CASE245 ASM("_op245:", "	.globl _op245"); case245
#define CASE246 ASM("_op246:", "	.globl _op246"); case246
#define CASE247 ASM("_op247:", "	.globl _op247"); case247
#define CASE250 ASM("_op250:", "	.globl _op250"); case250
#define CASE251 ASM("_op251:", "	.globl _op251"); case251
#define CASE252 ASM("_op252:", "	.globl _op252"); case252
#define CASE253 ASM("_op253:", "	.globl _op253"); case253
#define CASE254 ASM("_op254:", "	.globl _op254"); case254
#define CASE255 ASM("_op255:", "	.globl _op255"); case255
#define CASE256 ASM("_op256:", "	.globl _op256"); case256
#define CASE257 ASM("_op257:", "	.globl _op257"); case257
#define CASE260 ASM("_op260:", "	.globl _op260"); case260
#define CASE261 ASM("_op261:", "	.globl _op261"); case261
#define CASE262 ASM("_op262:", "	.globl _op262"); case262
#define CASE263 ASM("_op263:", "	.globl _op263"); case263
#define CASE264 ASM("_op264:", "	.globl _op264"); case264
#define CASE265 ASM("_op265:", "	.globl _op265"); case265
#define CASE266 ASM("_op266:", "	.globl _op266"); case266
#define CASE267 ASM("_op267:", "	.globl _op267"); case267
#define CASE270 ASM("_op270:", "	.globl _op270"); case270
#define CASE271 ASM("_op271:", "	.globl _op271"); case271
#define CASE272 ASM("_op272:", "	.globl _op272"); case272
#define CASE273 ASM("_op273:", "	.globl _op273"); case273
#define CASE274 ASM("_op274:", "	.globl _op274"); case274
#define CASE275 ASM("_op275:", "	.globl _op275"); case275
#define CASE276 ASM("_op276:", "	.globl _op276"); case276
#define CASE277 ASM("_op277:", "	.globl _op277"); case277
#define CASE300 ASM("_op300:", "	.globl _op300"); case300
#define CASE301 ASM("_op301:", "	.globl _op301"); case301
#define CASE302 ASM("_op302:", "	.globl _op302"); case302
#define CASE303 ASM("_op303:", "	.globl _op303"); case303
#define CASE304 ASM("_op304:", "	.globl _op304"); case304
#define CASE305 ASM("_op305:", "	.globl _op305"); case305
#define CASE306 ASM("_op306:", "	.globl _op306"); case306
#define CASE307 ASM("_op307:", "	.globl _op307"); case307
#define CASE310 ASM("_op310:", "	.globl _op310"); case310
#define CASE311 ASM("_op311:", "	.globl _op311"); case311
#define CASE312 ASM("_op312:", "	.globl _op312"); case312
#define CASE313 ASM("_op313:", "	.globl _op313"); case313
#define CASE314 ASM("_op314:", "	.globl _op314"); case314
#define CASE315 ASM("_op315:", "	.globl _op315"); case315
#define CASE316 ASM("_op316:", "	.globl _op316"); case316
#define CASE317 ASM("_op317:", "	.globl _op317"); case317
#define CASE320 ASM("_op320:", "	.globl _op320"); case320
#define CASE321 ASM("_op321:", "	.globl _op321"); case321
#define CASE322 ASM("_op322:", "	.globl _op322"); case322
#define CASE323 ASM("_op323:", "	.globl _op323"); case323
#define CASE324 ASM("_op324:", "	.globl _op324"); case324
#define CASE325 ASM("_op325:", "	.globl _op325"); case325
#define CASE326 ASM("_op326:", "	.globl _op326"); case326
#define CASE327 ASM("_op327:", "	.globl _op327"); case327
#define CASE330 ASM("_op330:", "	.globl _op330"); case330
#define CASE331 ASM("_op331:", "	.globl _op331"); case331
#define CASE332 ASM("_op332:", "	.globl _op332"); case332
#define CASE333 ASM("_op333:", "	.globl _op333"); case333
#define CASE334 ASM("_op334:", "	.globl _op334"); case334
#define CASE335 ASM("_op335:", "	.globl _op335"); case335
#define CASE336 ASM("_op336:", "	.globl _op336"); case336
#define CASE337 ASM("_op337:", "	.globl _op337"); case337
#define CASE340 ASM("_op340:", "	.globl _op340"); case340
#define CASE341 ASM("_op341:", "	.globl _op341"); case341
#define CASE342 ASM("_op342:", "	.globl _op342"); case342
#define CASE343 ASM("_op343:", "	.globl _op343"); case343
#define CASE344 ASM("_op344:", "	.globl _op344"); case344
#define CASE345 ASM("_op345:", "	.globl _op345"); case345
#define CASE346 ASM("_op346:", "	.globl _op346"); case346
#define CASE347 ASM("_op347:", "	.globl _op347"); case347
#define CASE350 ASM("_op350:", "	.globl _op350"); case350
#define CASE351 ASM("_op351:", "	.globl _op351"); case351
#define CASE352 ASM("_op352:", "	.globl _op352"); case352
#define CASE353 ASM("_op353:", "	.globl _op353"); case353
#define CASE354 ASM("_op354:", "	.globl _op354"); case354
#define CASE355 ASM("_op355:", "	.globl _op355"); case355
#define CASE356 ASM("_op356:", "	.globl _op356"); case356
#define CASE357 ASM("_op357:", "	.globl _op357"); case357
#define CASE360 ASM("_op360:", "	.globl _op360"); case360
#define CASE361 ASM("_op361:", "	.globl _op361"); case361
#define CASE362 ASM("_op362:", "	.globl _op362"); case362
#define CASE363 ASM("_op363:", "	.globl _op363"); case363
#define CASE364 ASM("_op364:", "	.globl _op364"); case364
#define CASE365 ASM("_op365:", "	.globl _op365"); case365
#define CASE366 ASM("_op366:", "	.globl _op366"); case366
#define CASE367 ASM("_op367:", "	.globl _op367"); case367
#define CASE370 ASM("_op370:", "	.globl _op370"); case370
#define CASE371 ASM("_op371:", "	.globl _op371"); case371
#define CASE372 ASM("_op372:", "	.globl _op372"); case372
#define CASE373 ASM("_op373:", "	.globl _op373"); case373
#define CASE374 ASM("_op374:", "	.globl _op374"); case374
#define CASE375 ASM("_op375:", "	.globl _op375"); case375
#define CASE376 ASM("_op376:", "	.globl _op376"); case376
#define CASE377 ASM("_op377:", "	.globl _op377"); case377
#endif /* PROFILE_H */
