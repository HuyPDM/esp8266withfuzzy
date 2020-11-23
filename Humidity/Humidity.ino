int value,real_value;
void setup() {
  Serial.begin(9600);                             //Mở cổng Serial ở tốc độ 9600
  pinMode(5,INPUT_PULLUP);                        // Mình sẽ dùng chân D1-GPIO5 để làm cổng đọc digital, còn chân A0 vào thì không cần khai báo cũng được nhé
}
 
void loop() {
  // Chúng ta sẽ tạo một hàm for để đọc 10 lần giá trị cảm biến, sau đó lấy giá trị trung bình để được giá trị chính xác nhất.
  for(int i=0;i<=9;i++){
    real_value+=analogRead(A0);
  }
  value=real_value/10;
  int percent = map(value, 350, 1023, 0, 100);    // Set giá thang giá trị đầu và giá trị cuối để đưa giá trị về thang từ 0-100. 
                                                  // Cái này sẽ bằng thực nghiệm nhé
  percent=100-percent;                            // Tính giá trị phần trăm thực. Chuyển điện thế từ 3.3V ( khô ) thành 3.3V ( ẩm )
  Serial.print(percent);
  Serial.print('%');
  Serial.print(" ");
  Serial.print("Gia tri analog: ");
  Serial.print(value);
  Serial.print(" ");
  Serial.print("Gia tri digital: ");
  Serial.println(digitalRead(5));
  delay(200);
  real_value=0;
}
