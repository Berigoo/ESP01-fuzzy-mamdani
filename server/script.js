// $(document).ready(function (e) {
//     $('#change-button').on('click', function (e) {
//         $('#img1').prop({src: "https://picsum.photos/500/300"})
//     });
// });
//
function refresh(msg){
    data = JSON.parse(msg);

    $('#temp-value').text(data['suhu']);
    $('#humidity-value').text(data['hum']);
    $('#curah-hujan-value').text(Math.round(data['curah_hujan']));


    $('#fuzzification-img1').prop({
        src: 'data:image/png;base64,' + data['Suhu.png']
    });
    $('#fuzzification-img2').prop({
        src: 'data:image/png;base64,' + data['Kelembaban.png']
    });
    $('#interference-img1').prop({
        src: 'data:image/png;base64,' + data['interference.png']
    });
    $('#defuzzification-img1').prop({
        src: 'data:image/png;base64,' + data['defuzzification.png']
    });
}
