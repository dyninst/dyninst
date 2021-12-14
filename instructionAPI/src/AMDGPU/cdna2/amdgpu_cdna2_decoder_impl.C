bool InstructionDecoder_amdgpu_cdna2::IS_ENC_SOP1(uint64_t I){
	switch( I & 4286643968 ){
	case 3196059648:
	case 3196059904:
	case 3196060672:
	case 3196060160:
	case 3196060416:
	case 3196060928:
	case 3196061184:
	case 3196061440:
	case 3196061696:
	case 3196061952:
	case 3196063744:
	case 3196064000:
	case 3196068096:
	case 3196067840:
	case 3196064256:
	case 3196068352:
	case 3196071936:
	case 3196072448:
	case 3196064512:
	case 3196068608:
	case 3196072704:
	case 3196064768:
	case 3196068864:
	case 3196072960:
	case 3196065024:
	case 3196069120:
	case 3196073216:
	case 3196065280:
	case 3196069376:
	case 3196073472:
	case 3196065536:
	case 3196069632:
	case 3196073728:
	case 3196065792:
	case 3196069888:
	case 3196066048:
	case 3196070144:
	case 3196062208:
	case 3196066304:
	case 3196070400:
	case 3196062464:
	case 3196066560:
	case 3196070656:
	case 3196062720:
	case 3196066816:
	case 3196070912:
	case 3196067584:
	case 3196062976:
	case 3196067072:
	case 3196071168:
	case 3196063232:
	case 3196067328:
	case 3196071424:
	case 3196063488:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_SOPC(uint64_t I){
	switch( I & 4294901760 ){
	case 3204448256:
	case 3204513792:
	case 3204644864:
	case 3204579328:
	case 3204710400:
	case 3204775936:
	case 3204841472:
	case 3204907008:
	case 3204972544:
	case 3205038080:
	case 3205103616:
	case 3205169152:
	case 3205234688:
	case 3205300224:
	case 3205365760:
	case 3205431296:
	case 3205693440:
	case 3205562368:
	case 3205627904:
	case 3205496832:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_SOPP(uint64_t I){
	switch( I & 4294901760 ){
	case 3212836864:
	case 3212902400:
	case 3213033472:
	case 3212967936:
	case 3213099008:
	case 3213164544:
	case 3213230080:
	case 3213295616:
	case 3213361152:
	case 3213426688:
	case 3213492224:
	case 3213623296:
	case 3213754368:
	case 3214016512:
	case 3214147584:
	case 3213885440:
	case 3214606336:
	case 3214737408:
	case 3214868480:
	case 3213557760:
	case 3213688832:
	case 3213819904:
	case 3214082048:
	case 3213950976:
	case 3214213120:
	case 3214278656:
	case 3214344192:
	case 3214475264:
	case 3214409728:
	case 3214540800:
	case 3214671872:
	case 3214802944:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_SOPK(uint64_t I){
	switch( I & 4286578688 ){
	case 2952790016:
	case 2961178624:
	case 2986344448:
	case 2969567232:
	case 2977955840:
	case 2994733056:
	case 3003121664:
	case 3011510272:
	case 3019898880:
	case 3028287488:
	case 3036676096:
	case 3045064704:
	case 3053453312:
	case 3061841920:
	case 3070230528:
	case 3078619136:
	case 3103784960:
	case 3095396352:
	case 3128950784:
	case 3120562176:
	case 3087007744:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_SOP2(uint64_t I){
	switch( I & 4286578688 ){
	case 2147483648:
	case 2155872256:
	case 2181038080:
	case 2164260864:
	case 2172649472:
	case 2189426688:
	case 2197815296:
	case 2206203904:
	case 2214592512:
	case 2222981120:
	case 2231369728:
	case 2239758336:
	case 2248146944:
	case 2256535552:
	case 2264924160:
	case 2273312768:
	case 2399141888:
	case 2533359616:
	case 2407530496:
	case 2541748224:
	case 2415919104:
	case 2550136832:
	case 2298478592:
	case 2306867200:
	case 2290089984:
	case 2315255808:
	case 2281701376:
	case 2323644416:
	case 2332033024:
	case 2340421632:
	case 2348810240:
	case 2357198848:
	case 2424307712:
	case 2558525440:
	case 2432696320:
	case 2566914048:
	case 2441084928:
	case 2575302656:
	case 2516582400:
	case 2449473536:
	case 2583691264:
	case 2457862144:
	case 2466250752:
	case 2474639360:
	case 2390753280:
	case 2483027968:
	case 2524971008:
	case 2491416576:
	case 2365587456:
	case 2499805184:
	case 2373976064:
	case 2508193792:
	case 2382364672:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_SMEM(uint64_t I){
	switch( I & 4294705152 ){
	case 3221225472:
	case 3221487616:
	case 3222274048:
	case 3221749760:
	case 3222011904:
	case 3222536192:
	case 3222798336:
	case 3223060480:
	case 3223322624:
	case 3223584768:
	case 3223846912:
	case 3224109056:
	case 3224371200:
	case 3225419776:
	case 3225681920:
	case 3225944064:
	case 3249537024:
	case 3229614080:
	case 3238002688:
	case 3246391296:
	case 3254779904:
	case 3257925632:
	case 3263168512:
	case 3229876224:
	case 3238264832:
	case 3246653440:
	case 3255042048:
	case 3263430656:
	case 3266314240:
	case 3230138368:
	case 3238526976:
	case 3246915584:
	case 3255304192:
	case 3263692800:
	case 3230400512:
	case 3238789120:
	case 3247177728:
	case 3255566336:
	case 3263954944:
	case 3230662656:
	case 3239051264:
	case 3247439872:
	case 3255828480:
	case 3264217088:
	case 3226730496:
	case 3230924800:
	case 3239313408:
	case 3247702016:
	case 3256090624:
	case 3264479232:
	case 3226992640:
	case 3231186944:
	case 3239575552:
	case 3247964160:
	case 3256352768:
	case 3264741376:
	case 3227254784:
	case 3231449088:
	case 3239837696:
	case 3248226304:
	case 3256614912:
	case 3265003520:
	case 3227516928:
	case 3231711232:
	case 3240099840:
	case 3248488448:
	case 3256877056:
	case 3265265664:
	case 3227779072:
	case 3231973376:
	case 3240361984:
	case 3248750592:
	case 3257139200:
	case 3265527808:
	case 3249274880:
	case 3228041216:
	case 3240624128:
	case 3249012736:
	case 3257401344:
	case 3265789952:
	case 3257663488:
	case 3240886272:
	case 3266052096:
	case 3241148416:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_VOP1(uint64_t I){
	switch( I & 4261543424 ){
	case 2113929216:
	case 2113929728:
	case 2113930752:
	case 2113931264:
	case 2113930240:
	case 2113931776:
	case 2113932288:
	case 2113932800:
	case 2113933312:
	case 2113934336:
	case 2113937408:
	case 2113937920:
	case 2113941504:
	case 2113942016:
	case 2113946112:
	case 2113950208:
	case 2113945600:
	case 2113954304:
	case 2113958400:
	case 2113962496:
	case 2113949696:
	case 2113953792:
	case 2113938432:
	case 2113942528:
	case 2113946624:
	case 2113950720:
	case 2113954816:
	case 2113957888:
	case 2113958912:
	case 2113961984:
	case 2113963008:
	case 2113966080:
	case 2113934848:
	case 2113938944:
	case 2113943040:
	case 2113947136:
	case 2113951232:
	case 2113955328:
	case 2113959424:
	case 2113963520:
	case 2113967616:
	case 2113935360:
	case 2113939456:
	case 2113943552:
	case 2113947648:
	case 2113951744:
	case 2113955840:
	case 2113959936:
	case 2113964032:
	case 2113968128:
	case 2113972224:
	case 2113967104:
	case 2113971200:
	case 2113935872:
	case 2113939968:
	case 2113944064:
	case 2113948160:
	case 2113952256:
	case 2113956352:
	case 2113960448:
	case 2113964544:
	case 2113968640:
	case 2113972736:
	case 2113936384:
	case 2113940480:
	case 2113944576:
	case 2113948672:
	case 2113952768:
	case 2113960960:
	case 2113965056:
	case 2113969152:
	case 2113973248:
	case 2113973760:
	case 2113936896:
	case 2113940992:
	case 2113945088:
	case 2113949184:
	case 2113953280:
	case 2113957376:
	case 2113961472:
	case 2113965568:
	case 2113969664:
	case 2113966592:
	case 2113970688:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_VOPC(uint64_t I){
	switch( I & 4294836224 ){
	case 2082471936:
	case 2082603008:
	case 2082865152:
	case 2082734080:
	case 2082996224:
	case 2083127296:
	case 2084569088:
	case 2084700160:
	case 2084831232:
	case 2084962304:
	case 2085093376:
	case 2085617664:
	case 2088239104:
	case 2092433408:
	case 2088763392:
	case 2092957696:
	case 2096627712:
	case 2113404928:
	case 2109341696:
	case 2105278464:
	case 2109865984:
	case 2105802752:
	case 2101739520:
	case 2110390272:
	case 2106327040:
	case 2102263808:
	case 2110914560:
	case 2090074112:
	case 2106851328:
	case 2086010880:
	case 2102788096:
	case 2094661632:
	case 2111438848:
	case 2090598400:
	case 2107375616:
	case 2086535168:
	case 2103312384:
	case 2095185920:
	case 2111963136:
	case 2091122688:
	case 2107899904:
	case 2087059456:
	case 2103836672:
	case 2095710208:
	case 2112487424:
	case 2108424192:
	case 2087583744:
	case 2104360960:
	case 2108948480:
	case 2096234496:
	case 2113011712:
	case 2085224448:
	case 2085748736:
	case 2088370176:
	case 2092564480:
	case 2088894464:
	case 2093088768:
	case 2089418752:
	case 2093613056:
	case 2089943040:
	case 2094137344:
	case 2092171264:
	case 2104885248:
	case 2096758784:
	case 2113536000:
	case 2109472768:
	case 2105409536:
	case 2101346304:
	case 2109997056:
	case 2105933824:
	case 2101870592:
	case 2110521344:
	case 2106458112:
	case 2102394880:
	case 2094268416:
	case 2111045632:
	case 2090205184:
	case 2106982400:
	case 2086141952:
	case 2102919168:
	case 2094792704:
	case 2111569920:
	case 2090729472:
	case 2107506688:
	case 2086666240:
	case 2103443456:
	case 2095316992:
	case 2112094208:
	case 2091253760:
	case 2108030976:
	case 2087190528:
	case 2103967744:
	case 2095841280:
	case 2112618496:
	case 2108555264:
	case 2087714816:
	case 2104492032:
	case 2113142784:
	case 2085355520:
	case 2087976960:
	case 2088501248:
	case 2092695552:
	case 2091646976:
	case 2089025536:
	case 2093219840:
	case 2089549824:
	case 2093744128:
	case 2085879808:
	case 2096365568:
	case 2105016320:
	case 2109079552:
	case 2096889856:
	case 2113667072:
	case 2109603840:
	case 2105540608:
	case 2101477376:
	case 2110128128:
	case 2089287680:
	case 2106064896:
	case 2102001664:
	case 2110652416:
	case 2089811968:
	case 2106589184:
	case 2102525952:
	case 2094399488:
	case 2111176704:
	case 2090336256:
	case 2107113472:
	case 2086273024:
	case 2103050240:
	case 2094923776:
	case 2111700992:
	case 2090860544:
	case 2107637760:
	case 2086797312:
	case 2103574528:
	case 2095448064:
	case 2112225280:
	case 2091384832:
	case 2108162048:
	case 2087321600:
	case 2104098816:
	case 2095972352:
	case 2112749568:
	case 2091909120:
	case 2108686336:
	case 2087845888:
	case 2104623104:
	case 2085486592:
	case 2088108032:
	case 2092302336:
	case 2088632320:
	case 2092826624:
	case 2089156608:
	case 2093350912:
	case 2091778048:
	case 2089680896:
	case 2093875200:
	case 2096496640:
	case 2109210624:
	case 2113273856:
	case 2105147392:
	case 2097020928:
	case 2113798144:
	case 2109734912:
	case 2105671680:
	case 2101608448:
	case 2104229888:
	case 2093481984:
	case 2110259200:
	case 2106195968:
	case 2102132736:
	case 2094006272:
	case 2110783488:
	case 2106720256:
	case 2102657024:
	case 2094530560:
	case 2111307776:
	case 2090467328:
	case 2107244544:
	case 2086404096:
	case 2103181312:
	case 2092040192:
	case 2112880640:
	case 2095054848:
	case 2111832064:
	case 2090991616:
	case 2107768832:
	case 2086928384:
	case 2103705600:
	case 2108817408:
	case 2095579136:
	case 2112356352:
	case 2091515904:
	case 2108293120:
	case 2087452672:
	case 2104754176:
	case 2096103424:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_VOP2(uint64_t I){
	switch( I & 4261412864 ){
	case 0:
	case 33554432:
	case 134217728:
	case 67108864:
	case 100663296:
	case 167772160:
	case 201326592:
	case 234881024:
	case 268435456:
	case 301989888:
	case 335544320:
	case 369098752:
	case 402653184:
	case 436207616:
	case 469762048:
	case 503316480:
	case 1174405120:
	case 1577058304:
	case 1308622848:
	case 1442840576:
	case 1073741824:
	case 1342177280:
	case 1476395008:
	case 603979776:
	case 637534208:
	case 671088640:
	case 570425344:
	case 536870912:
	case 704643072:
	case 838860800:
	case 872415232:
	case 905969664:
	case 939524096:
	case 1711276032:
	case 1845493760:
	case 1979711488:
	case 973078528:
	case 1107296256:
	case 1375731712:
	case 1509949440:
	case 1610612736:
	case 1644167168:
	case 1744830464:
	case 1778384896:
	case 1912602624:
	case 1879048192:
	case 2013265920:
	case 1006632960:
	case 1140850688:
	case 1275068416:
	case 1409286144:
	case 1543503872:
	case 1677721600:
	case 1811939328:
	case 1946157056:
	case 2046820352:
	case 1040187392:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_VINTRP(uint64_t I){
	switch( I & 4227858432 ){
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_VOP3P(uint64_t I){
	switch( I & 4294901760 ){
	case 3548381184:
	case 3548446720:
	case 3548577792:
	case 3548512256:
	case 3548643328:
	case 3548708864:
	case 3548774400:
	case 3548839936:
	case 3548905472:
	case 3548971008:
	case 3549036544:
	case 3549167616:
	case 3549298688:
	case 3549560832:
	case 3550478336:
	case 3549429760:
	case 3554213888:
	case 3551723520:
	case 3549102080:
	case 3549233152:
	case 3549364224:
	case 3550543872:
	case 3549495296:
	case 3550674944:
	case 3550937088:
	case 3551068160:
	case 3551199232:
	case 3551592448:
	case 3551657984:
	case 3554148352:
	case 3550609408:
	case 3550871552:
	case 3551002624:
	case 3551133696:
	case 3551526912:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_VOP3(uint64_t I){
	switch( I & 4294901760 ){
	case 3510632448:
	case 3510697984:
	case 3510829056:
	case 3510763520:
	case 3510894592:
	case 3510960128:
	case 3511025664:
	case 3511091200:
	case 3511156736:
	case 3511287808:
	case 3511353344:
	case 3514826752:
	case 3506438144:
	case 3523215360:
	case 3508535296:
	case 3491758080:
	case 3505913856:
	case 3504668672:
	case 3503423488:
	case 3496017920:
	case 3512795136:
	case 3514892288:
	case 3506503680:
	case 3531669504:
	case 3533766656:
	case 3523280896:
	case 3508600832:
	case 3491823616:
	case 3493920768:
	case 3500933120:
	case 3505520640:
	case 3519152128:
	case 3521249280:
	case 3514957824:
	case 3512860672:
	case 3506569216:
	case 3523346432:
	case 3508666368:
	case 3491889152:
	case 3493986304:
	case 3496083456:
	case 3504275456:
	case 3493855232:
	case 3503030272:
	case 3508731904:
	case 3491954688:
	case 3494051840:
	case 3496148992:
	case 3512926208:
	case 3515023360:
	case 3506634752:
	case 3531800576:
	case 3533897728:
	case 3519217664:
	case 3501785088:
	case 3506372608:
	case 3495952384:
	case 3500539904:
	case 3505127424:
	case 3531866112:
	case 3533963264:
	case 3519283200:
	case 3521380352:
	case 3515088896:
	case 3512991744:
	case 3506700288:
	case 3523477504:
	case 3492020224:
	case 3494117376:
	case 3503882240:
	case 3502637056:
	case 3501391872:
	case 3521445888:
	case 3506765824:
	case 3523543040:
	case 3492085760:
	case 3494182912:
	case 3496280064:
	case 3513057280:
	case 3515154432:
	case 3531931648:
	case 3534028800:
	case 3505979392:
	case 3500146688:
	case 3504734208:
	case 3503489024:
	case 3494248448:
	case 3496345600:
	case 3531997184:
	case 3519414272:
	case 3521511424:
	case 3515219968:
	case 3513122816:
	case 3506831360:
	case 3523608576:
	case 3508928512:
	case 3502243840:
	case 3532259328:
	case 3500998656:
	case 3505586176:
	case 3519479808:
	case 3521576960:
	case 3506896896:
	case 3523674112:
	case 3508994048:
	case 3492216832:
	case 3494313984:
	case 3496411136:
	case 3513188352:
	case 3515285504:
	case 3495165952:
	case 3504340992:
	case 3519348736:
	case 3503095808:
	case 3515351040:
	case 3506962432:
	case 3523739648:
	case 3509059584:
	case 3492282368:
	case 3494379520:
	case 3513253888:
	case 3496476672:
	case 3532128256:
	case 3519545344:
	case 3500605440:
	case 3505192960:
	case 3496542208:
	case 3513319424:
	case 3515416576:
	case 3532193792:
	case 3519610880:
	case 3507027968:
	case 3509125120:
	case 3492347904:
	case 3494445056:
	case 3500736512:
	case 3503947776:
	case 3502702592:
	case 3519676416:
	case 3521773568:
	case 3515482112:
	case 3513384960:
	case 3507093504:
	case 3509190656:
	case 3492413440:
	case 3494510592:
	case 3496607744:
	case 3491037184:
	case 3501457408:
	case 3506044928:
	case 3500212224:
	case 3504799744:
	case 3509256192:
	case 3492478976:
	case 3494576128:
	case 3496673280:
	case 3513450496:
	case 3515547648:
	case 3532324864:
	case 3519741952:
	case 3521839104:
	case 3507159040:
	case 3503554560:
	case 3512729600:
	case 3502309376:
	case 3501064192:
	case 3532390400:
	case 3519807488:
	case 3521904640:
	case 3511418880:
	case 3515613184:
	case 3513516032:
	case 3507224576:
	case 3509321728:
	case 3492544512:
	case 3494641664:
	case 3505651712:
	case 3504406528:
	case 3503161344:
	case 3521970176:
	case 3507290112:
	case 3509387264:
	case 3492610048:
	case 3494707200:
	case 3496804352:
	case 3513581568:
	case 3511484416:
	case 3515678720:
	case 3532455936:
	case 3500670976:
	case 3505258496:
	case 3494772736:
	case 3496869888:
	case 3492675584:
	case 3519938560:
	case 3522035712:
	case 3511549952:
	case 3515744256:
	case 3513647104:
	case 3509846016:
	case 3507355648:
	case 3502768128:
	case 3532783616:
	case 3522363392:
	case 3520004096:
	case 3522101248:
	case 3507421184:
	case 3509518336:
	case 3492741120:
	case 3494838272:
	case 3496935424:
	case 3513712640:
	case 3511615488:
	case 3515809792:
	case 3501522944:
	case 3506110464:
	case 3500277760:
	case 3504865280:
	case 3519873024:
	case 3509452800:
	case 3511681024:
	case 3507486720:
	case 3490709504:
	case 3509583872:
	case 3492806656:
	case 3494903808:
	case 3513778176:
	case 3497000960:
	case 3532652544:
	case 3520069632:
	case 3503620096:
	case 3502374912:
	case 3501129728:
	case 3497066496:
	case 3515940864:
	case 3513843712:
	case 3511746560:
	case 3532718080:
	case 3520135168:
	case 3494969344:
	case 3522232320:
	case 3507552256:
	case 3490775040:
	case 3505717248:
	case 3504472064:
	case 3503226880:
	case 3520200704:
	case 3522297856:
	case 3511812096:
	case 3513909248:
	case 3516006400:
	case 3507617792:
	case 3490840576:
	case 3509714944:
	case 3492937728:
	case 3495034880:
	case 3505324032:
	case 3509780480:
	case 3493003264:
	case 3495100416:
	case 3497197568:
	case 3513974784:
	case 3511877632:
	case 3532849152:
	case 3490906112:
	case 3507683328:
	case 3520266240:
	case 3504078848:
	case 3502833664:
	case 3532914688:
	case 3493068800:
	case 3520331776:
	case 3511943168:
	case 3514040320:
	case 3516137472:
	case 3522428928:
	case 3497263104:
	case 3507748864:
	case 3490971648:
	case 3506176000:
	case 3501326336:
	case 3531603968:
	case 3500343296:
	case 3504930816:
	case 3520397312:
	case 3495231488:
	case 3522494464:
	case 3507814400:
	case 3509911552:
	case 3493134336:
	case 3516203008:
	case 3512008704:
	case 3514105856:
	case 3497328640:
	case 3503685632:
	case 3533701120:
	case 3502440448:
	case 3501195264:
	case 3509977088:
	case 3493199872:
	case 3495297024:
	case 3533045760:
	case 3520462848:
	case 3512074240:
	case 3516268544:
	case 3522560000:
	case 3497394176:
	case 3501588480:
	case 3505782784:
	case 3504537600:
	case 3503292416:
	case 3492872192:
	case 3533111296:
	case 3520528384:
	case 3522625536:
	case 3510042624:
	case 3493265408:
	case 3495362560:
	case 3516334080:
	case 3512139776:
	case 3514236928:
	case 3497459712:
	case 3496214528:
	case 3532062720:
	case 3500802048:
	case 3505389568:
	case 3514302464:
	case 3522691072:
	case 3510108160:
	case 3493330944:
	case 3512205312:
	case 3495428096:
	case 3497525248:
	case 3533176832:
	case 3520593920:
	case 3501719552:
	case 3504144384:
	case 3502899200:
	case 3495493632:
	case 3512270848:
	case 3514368000:
	case 3497590784:
	case 3533242368:
	case 3520659456:
	case 3522756608:
	case 3510173696:
	case 3493396480:
	case 3501654016:
	case 3506241536:
	case 3500408832:
	case 3504996352:
	case 3520724992:
	case 3514433536:
	case 3512336384:
	case 3522822144:
	case 3510239232:
	case 3493462016:
	case 3495559168:
	case 3497656320:
	case 3533307904:
	case 3501850624:
	case 3503751168:
	case 3502505984:
	case 3501260800:
	case 3510304768:
	case 3493527552:
	case 3495624704:
	case 3512401920:
	case 3514499072:
	case 3497721856:
	case 3520790528:
	case 3522887680:
	case 3501916160:
	case 3504013312:
	case 3505848320:
	case 3504603136:
	case 3503357952:
	case 3497787392:
	case 3533438976:
	case 3520856064:
	case 3514564608:
	case 3512467456:
	case 3522953216:
	case 3510370304:
	case 3493593088:
	case 3495690240:
	case 3501981696:
	case 3500867584:
	case 3505455104:
	case 3520921600:
	case 3523018752:
	case 3510435840:
	case 3493658624:
	case 3495755776:
	case 3512532992:
	case 3514630144:
	case 3497852928:
	case 3533504512:
	case 3502047232:
	case 3504209920:
	case 3502964736:
	case 3497132032:
	case 3493724160:
	case 3495821312:
	case 3497918464:
	case 3533570048:
	case 3520987136:
	case 3514695680:
	case 3512598528:
	case 3532980224:
	case 3523084288:
	case 3502112768:
	case 3506307072:
	case 3531735040:
	case 3521314816:
	case 3500474368:
	case 3505061888:
	case 3509649408:
	case 3533635584:
	case 3521052672:
	case 3523149824:
	case 3508469760:
	case 3493789696:
	case 3495886848:
	case 3512664064:
	case 3514761216:
	case 3497984000:
	case 3502178304:
	case 3503816704:
	case 3533832192:
	case 3523411968:
	case 3502571520:
	case 3492151296:
	case 3496738816:
	case 3532587008:
	case 3522166784:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_DS(uint64_t I){
	switch( I & 4261281792 ){
	case 3623878656:
	case 3624009728:
	case 3624271872:
	case 3624140800:
	case 3624402944:
	case 3624534016:
	case 3624665088:
	case 3624796160:
	case 3624927232:
	case 3625058304:
	case 3625189376:
	case 3625451520:
	case 3631742976:
	case 3635412992:
	case 3628072960:
	case 3632267264:
	case 3640131584:
	case 3644194816:
	case 3648782336:
	case 3636592640:
	case 3637116928:
	case 3637641216:
	case 3638165504:
	case 3634102272:
	case 3630039040:
	case 3625975808:
	case 3638689792:
	case 3634626560:
	case 3630563328:
	case 3626500096:
	case 3635150848:
	case 3631087616:
	case 3647864832:
	case 3627024384:
	case 3643801600:
	case 3625582592:
	case 3627679744:
	case 3631874048:
	case 3631349760:
	case 3635544064:
	case 3628204032:
	case 3632398336:
	case 3628728320:
	case 3632922624:
	case 3629252608:
	case 3635675136:
	case 3644325888:
	case 3652976640:
	case 3636723712:
	case 3628597248:
	case 3637248000:
	case 3629121536:
	case 3637772288:
	case 3633709056:
	case 3629645824:
	case 3638296576:
	case 3634233344:
	case 3630170112:
	case 3626106880:
	case 3638820864:
	case 3634757632:
	case 3630694400:
	case 3626631168:
	case 3639345152:
	case 3635281920:
	case 3631218688:
	case 3647995904:
	case 3643932672:
	case 3625713664:
	case 3631480832:
	case 3627810816:
	case 3632005120:
	case 3628335104:
	case 3632529408:
	case 3628859392:
	case 3633053696:
	case 3629383680:
	case 3633577984:
	case 3635806208:
	case 3644456960:
	case 3640393728:
	case 3657170944:
	case 3653107712:
	case 3636854784:
	case 3632791552:
	case 3644063744:
	case 3637379072:
	case 3633315840:
	case 3637903360:
	case 3633840128:
	case 3629776896:
	case 3638427648:
	case 3634364416:
	case 3630301184:
	case 3626237952:
	case 3647733760:
	case 3638951936:
	case 3634888704:
	case 3630825472:
	case 3639476224:
	case 3625320448:
	case 3625844736:
	case 3631611904:
	case 3627941888:
	case 3632136192:
	case 3628466176:
	case 3632660480:
	case 3628990464:
	case 3633184768:
	case 3629514752:
	case 3635937280:
	case 3648651264:
	case 3657302016:
	case 3636461568:
	case 3636985856:
	case 3637510144:
	case 3633446912:
	case 3638034432:
	case 3633971200:
	case 3629907968:
	case 3638558720:
	case 3634495488:
	case 3630432256:
	case 3626369024:
	case 3635019776:
	case 3630956544:
	case 3626893312:
	case 3639607296:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_MUBUF(uint64_t I){
	switch( I & 4261150720 ){
	case 3758096384:
	case 3758358528:
	case 3759144960:
	case 3758620672:
	case 3758882816:
	case 3759407104:
	case 3759669248:
	case 3759931392:
	case 3760193536:
	case 3760455680:
	case 3760717824:
	case 3760979968:
	case 3761242112:
	case 3761504256:
	case 3761766400:
	case 3762028544:
	case 3765698560:
	case 3765960704:
	case 3778019328:
	case 3778543616:
	case 3766222848:
	case 3778805760:
	case 3786407936:
	case 3762290688:
	case 3766484992:
	case 3774873600:
	case 3779067904:
	case 3783262208:
	case 3762552832:
	case 3766747136:
	case 3775135744:
	case 3779330048:
	case 3783524352:
	case 3762814976:
	case 3767009280:
	case 3775397888:
	case 3783786496:
	case 3763077120:
	case 3767271424:
	case 3775660032:
	case 3778281472:
	case 3784048640:
	case 3763339264:
	case 3767533568:
	case 3775922176:
	case 3784310784:
	case 3763601408:
	case 3767795712:
	case 3776184320:
	case 3784572928:
	case 3763863552:
	case 3768057856:
	case 3776446464:
	case 3784835072:
	case 3764125696:
	case 3768320000:
	case 3776708608:
	case 3785097216:
	case 3764387840:
	case 3768582144:
	case 3776970752:
	case 3785359360:
	case 3777757184:
	case 3764649984:
	case 3768844288:
	case 3777232896:
	case 3785621504:
	case 3786145792:
	case 3764912128:
	case 3777495040:
	case 3785883648:
	case 3765174272:
	case 3765436416:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_MTBUF(uint64_t I){
	switch( I & 4228349952 ){
	case 3892314112:
	case 3892346880:
	case 3892412416:
	case 3892445184:
	case 3892379648:
	case 3892477952:
	case 3892510720:
	case 3892543488:
	case 3892576256:
	case 3892609024:
	case 3892641792:
	case 3892674560:
	case 3892707328:
	case 3892740096:
	case 3892772864:
	case 3892805632:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_MIMG(uint64_t I){
	switch( I & 4227858432 ){
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_FLAT(uint64_t I){
	switch( I & 4261199872 ){
	case 3695181824:
	case 3695443968:
	case 3696230400:
	case 3695706112:
	case 3695968256:
	case 3696492544:
	case 3696754688:
	case 3697016832:
	case 3697278976:
	case 3697541120:
	case 3697803264:
	case 3698065408:
	case 3698327552:
	case 3698589696:
	case 3698851840:
	case 3699113984:
	case 3711172608:
	case 3711434752:
	case 3711696896:
	case 3699376128:
	case 3707764736:
	case 3711959040:
	case 3716153344:
	case 3699638272:
	case 3708026880:
	case 3712221184:
	case 3716415488:
	case 3719036928:
	case 3699900416:
	case 3708289024:
	case 3712483328:
	case 3716677632:
	case 3700162560:
	case 3708551168:
	case 3716939776:
	case 3700424704:
	case 3708813312:
	case 3717201920:
	case 3700686848:
	case 3709075456:
	case 3717464064:
	case 3709337600:
	case 3717726208:
	case 3709599744:
	case 3717988352:
	case 3719299072:
	case 3709861888:
	case 3718250496:
	case 3710124032:
	case 3718512640:
	case 3710386176:
	case 3718774784:
	case 3710648320:
	case 3710910464:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_FLAT_GLBL(uint64_t I){
	switch( I & 4261199872 ){
	case 3695214592:
	case 3695476736:
	case 3696263168:
	case 3696001024:
	case 3695738880:
	case 3696525312:
	case 3696787456:
	case 3697049600:
	case 3697311744:
	case 3697573888:
	case 3697836032:
	case 3701768192:
	case 3710156800:
	case 3718545408:
	case 3702030336:
	case 3710418944:
	case 3718807552:
	case 3710681088:
	case 3719069696:
	case 3710943232:
	case 3719331840:
	case 3698098176:
	case 3698360320:
	case 3698622464:
	case 3698884608:
	case 3699146752:
	case 3699408896:
	case 3699671040:
	case 3699933184:
	case 3700457472:
	case 3700195328:
	case 3709632512:
	case 3711205376:
	case 3711467520:
	case 3718021120:
	case 3711729664:
	case 3707797504:
	case 3711991808:
	case 3716186112:
	case 3708059648:
	case 3712253952:
	case 3716448256:
	case 3701506048:
	case 3708321792:
	case 3712516096:
	case 3716710400:
	case 3709894656:
	case 3708583936:
	case 3716972544:
	case 3718283264:
	case 3708846080:
	case 3717234688:
	case 3700719616:
	case 3709108224:
	case 3717496832:
	case 3700981760:
	case 3709370368:
	case 3717758976:
	case 3701243904:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_FLAT_SCRATCH(uint64_t I){
	switch( I & 4261199872 ){
	case 3695198208:
	case 3695460352:
	case 3696246784:
	case 3695722496:
	case 3695984640:
	case 3696508928:
	case 3696771072:
	case 3697033216:
	case 3697295360:
	case 3697557504:
	case 3697819648:
	case 3698081792:
	case 3698343936:
	case 3698606080:
	case 3698868224:
	case 3699130368:
	case 3699392512:
	case 3699654656:
	case 3699916800:
	case 3700178944:
	case 3700441088:
	case 3700703232:
	case 3700965376:
	case 3701227520:
	case 3701489664:
	case 3701751808:
	case 3702013952:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_VOP2_LITERAL(uint64_t I){
	switch( I & 4261412864 ){
	case 771751936:
	case 805306368:
	case 1207959552:
	case 1241513984:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_VOP3B(uint64_t I){
	switch( I & 4294901760 ){
	case 3508076544:
	case 3508142080:
	case 3508338688:
	case 3508207616:
	case 3508273152:
	case 3508404224:
	case 3521118208:
	case 3521183744:
	case 3521642496:
	case 3521708032:
		return true;
	default:
		return false;
	}
}
bool InstructionDecoder_amdgpu_cdna2::IS_ENC_VOP3P_MFMA(uint64_t I){
	switch( I & 4294901760 ){
	case 3552444416:
	case 3552509952:
	case 3552641024:
	case 3552706560:
	case 3552575488:
	case 3552837632:
	case 3552903168:
	case 3553099776:
	case 3553165312:
	case 3553230848:
	case 3553361920:
	case 3553624064:
	case 3553755136:
	case 3554017280:
	case 3554541568:
	case 3554672640:
	case 3555983360:
	case 3556114432:
	case 3556245504:
	case 3556376576:
	case 3556507648:
	case 3556638720:
	case 3553427456:
	case 3553689600:
	case 3554082816:
	case 3554476032:
	case 3554607104:
	case 3554738176:
	case 3555655680:
	case 3555786752:
	case 3555917824:
	case 3555852288:
	case 3556048896:
	case 3556179968:
	case 3556311040:
	case 3556442112:
	case 3556573184:
	case 3556704256:
	case 3554803712:
	case 3554934784:
	case 3555065856:
	case 3555196928:
	case 3555328000:
	case 3555459072:
	case 3555590144:
	case 3555721216:
		return true;
	default:
		return false;
	}
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_SOP1(){
	insn_size = 4;
	layout_ENC_SOP1 & layout = insn_layout.ENC_SOP1;
	layout.ENCODING = longfield<23,31>(insn_long);
	layout.OP = longfield<8,15>(insn_long);
	layout.SDST = longfield<16,22>(insn_long);
	layout.SSRC0 = longfield<0,7>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOP1_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOP1Operands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_SOPC(){
	insn_size = 4;
	layout_ENC_SOPC & layout = insn_layout.ENC_SOPC;
	layout.ENCODING = longfield<23,31>(insn_long);
	layout.OP = longfield<16,22>(insn_long);
	layout.SSRC0 = longfield<0,7>(insn_long);
	layout.SSRC1 = longfield<8,15>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOPC_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOPCOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_SOPP(){
	insn_size = 4;
	layout_ENC_SOPP & layout = insn_layout.ENC_SOPP;
	layout.ENCODING = longfield<23,31>(insn_long);
	layout.OP = longfield<16,22>(insn_long);
	layout.SIMM16 = longfield<0,15>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOPP_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOPPOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_SOPK(){
	insn_size = 4;
	layout_ENC_SOPK & layout = insn_layout.ENC_SOPK;
	layout.ENCODING = longfield<28,31>(insn_long);
	layout.OP = longfield<23,27>(insn_long);
	layout.SDST = longfield<16,22>(insn_long);
	layout.SIMM16 = longfield<0,15>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOPK_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOPKOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_SOP2(){
	insn_size = 4;
	layout_ENC_SOP2 & layout = insn_layout.ENC_SOP2;
	layout.ENCODING = longfield<30,31>(insn_long);
	layout.OP = longfield<23,29>(insn_long);
	layout.SDST = longfield<16,22>(insn_long);
	layout.SSRC0 = longfield<0,7>(insn_long);
	layout.SSRC1 = longfield<8,15>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOP2_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SOP2Operands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_SMEM(){
	insn_size = 8;
	layout_ENC_SMEM & layout = insn_layout.ENC_SMEM;
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.GLC = longfield<16,16>(insn_long);
	layout.IMM = longfield<17,17>(insn_long);
	layout.NV = longfield<15,15>(insn_long);
	layout.OFFSET = longfield<32,52>(insn_long);
	layout.OP = longfield<18,25>(insn_long);
	layout.SBASE = longfield<0,5>(insn_long);
	layout.SDATA = longfield<6,12>(insn_long);
	layout.SOFFSET = longfield<57,63>(insn_long);
	layout.SOFFSET_EN = longfield<14,14>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SMEM_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_SMEMOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_VOP1(){
	insn_size = 4;
	layout_ENC_VOP1 & layout = insn_layout.ENC_VOP1;
	layout.ENCODING = longfield<25,31>(insn_long);
	layout.OP = longfield<9,16>(insn_long);
	layout.SRC0 = longfield<0,8>(insn_long);
	layout.VDST = longfield<17,24>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP1_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP1Operands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_VOPC(){
	insn_size = 4;
	layout_ENC_VOPC & layout = insn_layout.ENC_VOPC;
	layout.ENCODING = longfield<25,31>(insn_long);
	layout.OP = longfield<17,24>(insn_long);
	layout.SRC0 = longfield<0,8>(insn_long);
	layout.VSRC1 = longfield<9,16>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOPC_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOPCOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_VOP2(){
	insn_size = 4;
	layout_ENC_VOP2 & layout = insn_layout.ENC_VOP2;
	layout.ENCODING = longfield<31,31>(insn_long);
	layout.OP = longfield<25,30>(insn_long);
	layout.SRC0 = longfield<0,8>(insn_long);
	layout.VDST = longfield<17,24>(insn_long);
	layout.VSRC1 = longfield<9,16>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP2_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP2Operands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_VINTRP(){
	insn_size = 4;
	layout_ENC_VINTRP & layout = insn_layout.ENC_VINTRP;
	layout.ATTR = longfield<10,15>(insn_long);
	layout.ATTRCHAN = longfield<8,9>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.OP = longfield<16,17>(insn_long);
	layout.VDST = longfield<18,25>(insn_long);
	layout.VSRC = longfield<0,7>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VINTRP_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VINTRPOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_VOP3P(){
	insn_size = 8;
	layout_ENC_VOP3P & layout = insn_layout.ENC_VOP3P;
	layout.CLAMP = longfield<15,15>(insn_long);
	layout.ENCODING = longfield<23,31>(insn_long);
	layout.NEG = longfield<61,63>(insn_long);
	layout.NEG_HI = longfield<8,10>(insn_long);
	layout.OP = longfield<16,22>(insn_long);
	layout.OP_SEL = longfield<11,13>(insn_long);
	layout.OP_SEL_HI = longfield<59,60>(insn_long);
	layout.OP_SEL_HI_2 = longfield<14,14>(insn_long);
	layout.SRC0 = longfield<32,40>(insn_long);
	layout.SRC1 = longfield<41,49>(insn_long);
	layout.SRC2 = longfield<50,58>(insn_long);
	layout.VDST = longfield<0,7>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP3P_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP3POperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_VOP3(){
	insn_size = 8;
	layout_ENC_VOP3 & layout = insn_layout.ENC_VOP3;
	layout.ABS = longfield<8,10>(insn_long);
	layout.CLAMP = longfield<15,15>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.NEG = longfield<61,63>(insn_long);
	layout.OMOD = longfield<59,60>(insn_long);
	layout.OP = longfield<16,25>(insn_long);
	layout.OP_SEL = longfield<11,14>(insn_long);
	layout.SRC0 = longfield<32,40>(insn_long);
	layout.SRC1 = longfield<41,49>(insn_long);
	layout.SRC2 = longfield<50,58>(insn_long);
	layout.VDST = longfield<0,7>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP3_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP3Operands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_DS(){
	insn_size = 8;
	layout_ENC_DS & layout = insn_layout.ENC_DS;
	layout.ACC = longfield<25,25>(insn_long);
	layout.ADDR = longfield<32,39>(insn_long);
	layout.DATA0 = longfield<40,47>(insn_long);
	layout.DATA1 = longfield<48,55>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.GDS = longfield<16,16>(insn_long);
	layout.OFFSET0 = longfield<0,7>(insn_long);
	layout.OFFSET1 = longfield<8,15>(insn_long);
	layout.OP = longfield<17,24>(insn_long);
	layout.VDST = longfield<56,63>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_DS_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_DSOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_MUBUF(){
	insn_size = 8;
	layout_ENC_MUBUF & layout = insn_layout.ENC_MUBUF;
	layout.ACC = longfield<55,55>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.IDXEN = longfield<13,13>(insn_long);
	layout.LDS = longfield<16,16>(insn_long);
	layout.NT = longfield<17,17>(insn_long);
	layout.OFFEN = longfield<12,12>(insn_long);
	layout.OFFSET = longfield<0,11>(insn_long);
	layout.OP = longfield<18,24>(insn_long);
	layout.SC0 = longfield<14,14>(insn_long);
	layout.SC1 = longfield<15,15>(insn_long);
	layout.SOFFSET = longfield<56,63>(insn_long);
	layout.SRSRC = longfield<48,52>(insn_long);
	layout.VADDR = longfield<32,39>(insn_long);
	layout.VDATA = longfield<40,47>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_MUBUF_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_MUBUFOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_MTBUF(){
	insn_size = 8;
	layout_ENC_MTBUF & layout = insn_layout.ENC_MTBUF;
	layout.ACC = longfield<55,55>(insn_long);
	layout.DFMT = longfield<19,22>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.IDXEN = longfield<13,13>(insn_long);
	layout.NFMT = longfield<23,25>(insn_long);
	layout.NT = longfield<54,54>(insn_long);
	layout.OFFEN = longfield<12,12>(insn_long);
	layout.OFFSET = longfield<0,11>(insn_long);
	layout.OP = longfield<15,18>(insn_long);
	layout.SC0 = longfield<14,14>(insn_long);
	layout.SC1 = longfield<53,53>(insn_long);
	layout.SOFFSET = longfield<56,63>(insn_long);
	layout.SRSRC = longfield<48,52>(insn_long);
	layout.VADDR = longfield<32,39>(insn_long);
	layout.VDATA = longfield<40,47>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_MTBUF_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_MTBUFOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_MIMG(){
	insn_size = 8;
	layout_ENC_MIMG & layout = insn_layout.ENC_MIMG;
	layout.A16 = longfield<15,15>(insn_long);
	layout.ACC = longfield<16,16>(insn_long);
	layout.D16 = longfield<63,63>(insn_long);
	layout.DA = longfield<14,14>(insn_long);
	layout.DMASK = longfield<8,11>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.LWE = longfield<17,17>(insn_long);
	layout.NT = longfield<25,25>(insn_long);
	layout.OP = longfield<18,24>(insn_long);
	layout.OPM = longfield<0,0>(insn_long);
	layout.SC0 = longfield<13,13>(insn_long);
	layout.SC1 = longfield<7,7>(insn_long);
	layout.SRSRC = longfield<48,52>(insn_long);
	layout.SSAMP = longfield<53,57>(insn_long);
	layout.UNORM = longfield<12,12>(insn_long);
	layout.VADDR = longfield<32,39>(insn_long);
	layout.VDATA = longfield<40,47>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_MIMG_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_MIMGOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_FLAT(){
	insn_size = 8;
	layout_ENC_FLAT & layout = insn_layout.ENC_FLAT;
	layout.ACC = longfield<55,55>(insn_long);
	layout.ADDR = longfield<32,39>(insn_long);
	layout.DATA = longfield<40,47>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.NT = longfield<17,17>(insn_long);
	layout.OFFSET = longfield<0,11>(insn_long);
	layout.OP = longfield<18,24>(insn_long);
	layout.SADDR = longfield<48,54>(insn_long);
	layout.SC0 = longfield<16,16>(insn_long);
	layout.SC1 = longfield<25,25>(insn_long);
	layout.SEG = longfield<14,15>(insn_long);
	layout.SVE = longfield<13,13>(insn_long);
	layout.VDST = longfield<56,63>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_FLAT_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_FLATOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_FLAT_GLBL(){
	insn_size = 8;
	layout_ENC_FLAT_GLBL & layout = insn_layout.ENC_FLAT_GLBL;
	layout.ACC = longfield<55,55>(insn_long);
	layout.ADDR = longfield<32,39>(insn_long);
	layout.DATA = longfield<40,47>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.NT = longfield<17,17>(insn_long);
	layout.OFFSET = longfield<0,12>(insn_long);
	layout.OP = longfield<18,24>(insn_long);
	layout.SADDR = longfield<48,54>(insn_long);
	layout.SC0 = longfield<16,16>(insn_long);
	layout.SC1 = longfield<25,25>(insn_long);
	layout.SEG = longfield<14,15>(insn_long);
	layout.SVE = longfield<13,13>(insn_long);
	layout.VDST = longfield<56,63>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_FLAT_GLBL_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_FLAT_GLBLOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_FLAT_SCRATCH(){
	insn_size = 8;
	layout_ENC_FLAT_SCRATCH & layout = insn_layout.ENC_FLAT_SCRATCH;
	layout.ACC = longfield<55,55>(insn_long);
	layout.ADDR = longfield<32,39>(insn_long);
	layout.DATA = longfield<40,47>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.NT = longfield<17,17>(insn_long);
	layout.OFFSET = longfield<0,12>(insn_long);
	layout.OP = longfield<18,24>(insn_long);
	layout.SADDR = longfield<48,54>(insn_long);
	layout.SC0 = longfield<16,16>(insn_long);
	layout.SC1 = longfield<25,25>(insn_long);
	layout.SEG = longfield<14,15>(insn_long);
	layout.SVE = longfield<13,13>(insn_long);
	layout.VDST = longfield<56,63>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_FLAT_SCRATCH_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_FLAT_SCRATCHOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_VOP2_LITERAL(){
	insn_size = 8;
	layout_ENC_VOP2_LITERAL & layout = insn_layout.ENC_VOP2_LITERAL;
	layout.ENCODING = longfield<31,31>(insn_long);
	layout.OP = longfield<25,30>(insn_long);
	layout.SIMM32 = longfield<32,63>(insn_long);
	layout.SRC0 = longfield<0,8>(insn_long);
	layout.VDST = longfield<17,24>(insn_long);
	layout.VSRC1 = longfield<9,16>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP2_LITERAL_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP2_LITERALOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_VOP3B(){
	insn_size = 8;
	layout_ENC_VOP3B & layout = insn_layout.ENC_VOP3B;
	layout.CLAMP = longfield<15,15>(insn_long);
	layout.ENCODING = longfield<26,31>(insn_long);
	layout.NEG = longfield<61,63>(insn_long);
	layout.OMOD = longfield<59,60>(insn_long);
	layout.OP = longfield<16,25>(insn_long);
	layout.SDST = longfield<8,14>(insn_long);
	layout.SRC0 = longfield<32,40>(insn_long);
	layout.SRC1 = longfield<41,49>(insn_long);
	layout.SRC2 = longfield<50,58>(insn_long);
	layout.VDST = longfield<0,7>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP3B_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP3BOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::decodeENC_VOP3P_MFMA(){
	insn_size = 8;
	layout_ENC_VOP3P_MFMA & layout = insn_layout.ENC_VOP3P_MFMA;
	layout.ABID = longfield<11,14>(insn_long);
	layout.ACC = longfield<59,60>(insn_long);
	layout.ACC_CD = longfield<15,15>(insn_long);
	layout.BLGP = longfield<61,63>(insn_long);
	layout.CBSZ = longfield<8,10>(insn_long);
	layout.ENCODING = longfield<23,31>(insn_long);
	layout.OP = longfield<16,22>(insn_long);
	layout.SRC0 = longfield<32,40>(insn_long);
	layout.SRC1 = longfield<41,49>(insn_long);
	layout.SRC2 = longfield<50,58>(insn_long);
	layout.VDST = longfield<0,7>(insn_long);
	const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP3P_MFMA_insn_table[layout.OP];
	this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
	finalizeENC_VOP3P_MFMAOperands();
	this->insn_in_progress->updateSize(insn_size + immLen);
}
void InstructionDecoder_amdgpu_cdna2::mainDecodeOpcode(){
	if(IS_ENC_SOP1(insn_long)){
		insn_size = 4;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOP1_insn_table[longfield<8,15>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOP1;
	}
	else 	if(IS_ENC_SOPC(insn_long)){
		insn_size = 4;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOPC_insn_table[longfield<16,22>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOPC;
	}
	else 	if(IS_ENC_SOPP(insn_long)){
		insn_size = 4;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOPP_insn_table[longfield<16,22>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOPP;
	}
	else 	if(IS_ENC_SOPK(insn_long)){
		insn_size = 4;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOPK_insn_table[longfield<23,27>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOPK;
	}
	else 	if(IS_ENC_SOP2(insn_long)){
		insn_size = 4;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SOP2_insn_table[longfield<23,29>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SOP2;
	}
	else 	if(IS_ENC_SMEM(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_SMEM_insn_table[longfield<18,25>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_SMEM;
	}
	else 	if(IS_ENC_VOP1(insn_long)){
		insn_size = 4;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP1_insn_table[longfield<9,16>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP1;
	}
	else 	if(IS_ENC_VOPC(insn_long)){
		insn_size = 4;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOPC_insn_table[longfield<17,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOPC;
	}
	else 	if(IS_ENC_VOP2(insn_long)){
		insn_size = 4;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP2_insn_table[longfield<25,30>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP2;
	}
	else 	if(IS_ENC_VINTRP(insn_long)){
		insn_size = 4;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VINTRP_insn_table[longfield<16,17>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VINTRP;
	}
	else 	if(IS_ENC_VOP3P(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP3P_insn_table[longfield<16,22>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP3P;
	}
	else 	if(IS_ENC_VOP3(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP3_insn_table[longfield<16,25>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP3;
	}
	else 	if(IS_ENC_DS(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_DS_insn_table[longfield<17,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_DS;
	}
	else 	if(IS_ENC_MUBUF(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_MUBUF_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_MUBUF;
	}
	else 	if(IS_ENC_MTBUF(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_MTBUF_insn_table[longfield<15,18>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_MTBUF;
	}
	else 	if(IS_ENC_MIMG(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_MIMG_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_MIMG;
	}
	else 	if(IS_ENC_FLAT(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_FLAT_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_FLAT;
	}
	else 	if(IS_ENC_FLAT_GLBL(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_FLAT_GLBL_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_FLAT_GLBL;
	}
	else 	if(IS_ENC_FLAT_SCRATCH(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_FLAT_SCRATCH_insn_table[longfield<18,24>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_FLAT_SCRATCH;
	}
	else 	if(IS_ENC_VOP2_LITERAL(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP2_LITERAL_insn_table[longfield<25,30>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP2_LITERAL;
	}
	else 	if(IS_ENC_VOP3B(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP3B_insn_table[longfield<16,25>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP3B;
	}
	else 	if(IS_ENC_VOP3P_MFMA(insn_long)){
		insn_size = 8;
		const amdgpu_cdna2_insn_entry &insn_entry = amdgpu_cdna2_insn_entry::ENC_VOP3P_MFMA_insn_table[longfield<16,22>(insn_long)];
		this->insn_in_progress = makeInstruction(insn_entry.op,insn_entry.mnemonic,insn_size+immLen,reinterpret_cast<unsigned char *>(&insn));
		instr_family = ENC_VOP3P_MFMA;
	}
}
void InstructionDecoder_amdgpu_cdna2::mainDecode(){
	if(IS_ENC_SOP1(insn_long)){
		decodeENC_SOP1();
	}
	else 	if(IS_ENC_SOPC(insn_long)){
		decodeENC_SOPC();
	}
	else 	if(IS_ENC_SOPP(insn_long)){
		decodeENC_SOPP();
	}
	else 	if(IS_ENC_SOPK(insn_long)){
		decodeENC_SOPK();
	}
	else 	if(IS_ENC_SOP2(insn_long)){
		decodeENC_SOP2();
	}
	else 	if(IS_ENC_SMEM(insn_long)){
		decodeENC_SMEM();
	}
	else 	if(IS_ENC_VOP1(insn_long)){
		decodeENC_VOP1();
	}
	else 	if(IS_ENC_VOPC(insn_long)){
		decodeENC_VOPC();
	}
	else 	if(IS_ENC_VOP2(insn_long)){
		decodeENC_VOP2();
	}
	else 	if(IS_ENC_VINTRP(insn_long)){
		decodeENC_VINTRP();
	}
	else 	if(IS_ENC_VOP3P(insn_long)){
		decodeENC_VOP3P();
	}
	else 	if(IS_ENC_VOP3(insn_long)){
		decodeENC_VOP3();
	}
	else 	if(IS_ENC_DS(insn_long)){
		decodeENC_DS();
	}
	else 	if(IS_ENC_MUBUF(insn_long)){
		decodeENC_MUBUF();
	}
	else 	if(IS_ENC_MTBUF(insn_long)){
		decodeENC_MTBUF();
	}
	else 	if(IS_ENC_MIMG(insn_long)){
		decodeENC_MIMG();
	}
	else 	if(IS_ENC_FLAT(insn_long)){
		decodeENC_FLAT();
	}
	else 	if(IS_ENC_FLAT_GLBL(insn_long)){
		decodeENC_FLAT_GLBL();
	}
	else 	if(IS_ENC_FLAT_SCRATCH(insn_long)){
		decodeENC_FLAT_SCRATCH();
	}
	else 	if(IS_ENC_VOP2_LITERAL(insn_long)){
		decodeENC_VOP2_LITERAL();
	}
	else 	if(IS_ENC_VOP3B(insn_long)){
		decodeENC_VOP3B();
	}
	else 	if(IS_ENC_VOP3P_MFMA(insn_long)){
		decodeENC_VOP3P_MFMA();
	}
}
