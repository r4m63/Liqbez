# Java Stream API — Cheatsheet

## Что такое Stream?

**Stream** (`java.util.stream.Stream`) — последовательность элементов, над которой можно производить цепочку операций.

- **Не хранит данные** — это не коллекция, а конвейер обработки
- **Ленивый** — промежуточные операции не выполняются до вызова терминального метода
- **Одноразовый** — после терминальной операции стрим нельзя переиспользовать
- **Не изменяет источник** — исходная коллекция остаётся нетронутой

```
Источник → [промежуточные операции]* → терминальная операция
  ↓              ↓                            ↓
Collection     filter, map, ...           collect, count, ...
  (ленивые — не выполняются сразу)      (запускает весь конвейер)
```

---

## Создание стрима

```java
// Из коллекции
list.stream()
set.stream()

// Из массива
Arrays.stream(array)

// Из указанных элементов
Stream.of("a1", "a2", "a3")

// Пустой стрим
Stream.empty()

// Из Map
map.entrySet().stream()
map.keySet().stream()

// Из строки (символы → IntStream)
"hello".chars()                         // IntStream

// Из файла (строки)
Files.lines(Path.of("file.txt"))        // Stream<String>
new BufferedReader(reader).lines()

// Бесконечный стрим
Stream.iterate(0, n -> n + 1)           // 0, 1, 2, 3, ...
Stream.generate(Math::random)           // случайные числа

// Из диапазона (только примитивные)
IntStream.range(0, 10)                  // [0, 10)
IntStream.rangeClosed(1, 10)            // [1, 10]

// Конкатенация стримов
Stream.concat(stream1, stream2)
```

---

## Промежуточные операции (lazy)

> Возвращают новый Stream. Не выполняются до вызова терминального метода.

| Метод      | Сигнатура                         | Описание                                                      |
| ---------- | --------------------------------- | ------------------------------------------------------------- |
| `filter`   | `filter(Predicate<T>)`            | Оставляет только элементы, прошедшие условие                  |
| `map`      | `map(Function<T,R>)`              | Преобразует каждый элемент T → R                              |
| `flatMap`  | `flatMap(Function<T, Stream<R>>)` | Преобразует T → Stream<R>, затем «разворачивает» в один поток |
| `peek`     | `peek(Consumer<T>)`               | Выполняет действие над каждым элементом без изменения стрима  |
| `distinct` | `distinct()`                      | Убирает дубликаты (по `equals`)                               |
| `sorted`   | `sorted()` / `sorted(Comparator)` | Сортирует элементы                                            |
| `limit`    | `limit(long n)`                   | Оставляет первые n элементов                                  |
| `skip`     | `skip(long n)`                    | Пропускает первые n элементов                                 |
| `mapToInt` | `mapToInt(ToIntFunction<T>)`      | Преобразует в `IntStream`                                     |
| `mapToObj` | `mapToObj(IntFunction<R>)`        | IntStream → Stream<R>                                         |

### map() vs flatMap()

```java
// map: каждый элемент → один элемент
// Stream<List<String>> → Stream<String> НЕ получится через map
List<List<String>> nested = List.of(List.of("a","b"), List.of("c","d"));
nested.stream()
      .map(List::stream)       // Stream<Stream<String>> — НЕ то что нужно
      .flatMap(List::stream)   // Stream<String> — "разворачивает" вложенные стримы ✅

// Практический пример:
Stream.of("Hello World", "Java Stream")
      .flatMap(s -> Arrays.stream(s.split(" ")))
      // → "Hello", "World", "Java", "Stream"
```

### peek() — для отладки

```java
list.stream()
    .filter(x -> x > 0)
    .peek(x -> System.out.println("after filter: " + x))  // отладка
    .map(x -> x * 2)
    .peek(x -> System.out.println("after map: " + x))
    .collect(Collectors.toList());
```

> `peek()` — частный случай `map()`, но не меняет тип и элементы. Возвращает тот же объект.

---

## Терминальные операции (eager)

> Запускают весь конвейер. После вызова — стрим закрыт. Все "оконечные" методы возвращают `Optional` (чтобы не возвращать `null`).

| Метод                      | Возвращает          | Описание                                       |
| -------------------------- | ------------------- | ---------------------------------------------- |
| `collect(Collector)`       | `R`                 | Собирает элементы в коллекцию/структуру        |
| `forEach(Consumer)`        | `void`              | Выполняет действие для каждого элемента        |
| `forEachOrdered(Consumer)` | `void`              | `forEach`, но гарантирует порядок              |
| `count()`                  | `long`              | Подсчёт элементов                              |
| `findFirst()`              | `Optional<T>`       | Первый элемент                                 |
| `findAny()`                | `Optional<T>`       | Любой элемент (быстрее в параллельных стримах) |
| `min(Comparator)`          | `Optional<T>`       | Минимальный элемент                            |
| `max(Comparator)`          | `Optional<T>`       | Максимальный элемент                           |
| `anyMatch(Predicate)`      | `boolean`           | Хотя бы один элемент соответствует условию     |
| `allMatch(Predicate)`      | `boolean`           | Все элементы соответствуют условию             |
| `noneMatch(Predicate)`     | `boolean`           | Ни один элемент не соответствует условию       |
| `reduce(...)`              | `Optional<T>` / `T` | Свёртка элементов бинарным оператором          |
| `toArray()`                | `Object[]`          | Преобразует в массив                           |

---

## reduce()

Применяет бинарный оператор к парам элементов, пока не останется один результат.

```java
// 3 перегрузки:

// 1. Без начального значения → Optional (стрим может быть пустым)
Optional<T> reduce(BinaryOperator<T> accumulator)

// 2. С начальным значением → T (гарантированный результат)
T reduce(T identity, BinaryOperator<T> accumulator)

// 3. С combiner (для параллельных стримов, если типы T и U разные)
U reduce(U identity, BiFunction<U,? super T,U> accumulator, BinaryOperator<U> combiner)
```

```java
// Пример: сумма
int sum = IntStream.rangeClosed(1, 5)
                   .reduce(0, Integer::sum);  // 0+1+2+3+4+5 = 15

// Пример: конкатенация
String result = Stream.of("a", "b", "c")
                      .reduce("", (a, b) -> a + b);  // "abc"
```

---

## collect() и Collectors

`collect()` — терминальный метод для сворачивания стрима в коллекцию или агрегат.

```java
// → List
List<String> list = stream.collect(Collectors.toList());
List<String> list = stream.collect(Collectors.toUnmodifiableList()); // Java 10+

// → Set
Set<String> set = stream.collect(Collectors.toSet());

// → Map
Map<Integer, String> map = stream.collect(
    Collectors.toMap(String::length, s -> s)
);

// → String (join)
String joined = stream.collect(Collectors.joining(", ", "[", "]"));
// ["a", "b", "c"]

// → группировка
Map<Integer, List<String>> byLength = stream.collect(
    Collectors.groupingBy(String::length)
);

// → разбивка на 2 группы по предикату
Map<Boolean, List<Integer>> partition = stream.collect(
    Collectors.partitioningBy(x -> x % 2 == 0)
);

// → подсчёт в группах
Map<String, Long> counting = stream.collect(
    Collectors.groupingBy(s -> s, Collectors.counting())
);

// → статистика
IntSummaryStatistics stats = intStream.collect(
    Collectors.summarizingInt(Integer::intValue)
);
// stats.getCount(), getSum(), getMin(), getMax(), getAverage()
```

---

## Параллельные стримы

```java
// Создание:
list.parallelStream()
stream.parallel()          // обычный стрим → параллельный
stream.sequential()        // параллельный → последовательный
```

- Использует **Fork/Join Pool** (`ForkJoinPool.commonPool()`)
- Количество потоков = количество ядер процессора
- Если машина однопоточная — выполняется как последовательный

**Когда параллельный стрим быстрее:**
- Большой объём данных
- Независимые операции над элементами
- Нет общего изменяемого состояния

**Когда НЕ стоит использовать:**
- Маленькие коллекции (накладные расходы > выигрыш)
- Операции с порядком (`forEachOrdered`, `findFirst`)
- Операции с изменяемым состоянием (thread-safety проблемы)

---

## Примитивные стримы

Созданы для избежания boxing/unboxing. Работают быстрее, чем `Stream<Integer>`.

| Стрим          | Тип элементов | Доп. терминальные методы                                      |
| -------------- | ------------- | ------------------------------------------------------------- |
| `IntStream`    | `int`         | `sum()`, `average()`, `min()`, `max()`, `summaryStatistics()` |
| `LongStream`   | `long`        | `sum()`, `average()`, `min()`, `max()`, `summaryStatistics()` |
| `DoubleStream` | `double`      | `sum()`, `average()`, `min()`, `max()`, `summaryStatistics()` |

```java
IntStream.range(1, 6).sum()             // 15
IntStream.of(1, 2, 3).average()         // OptionalDouble[2.0]
IntStream.rangeClosed(1, 100).sum()     // 5050

// Конвертации
stream.mapToInt(String::length)         // Stream<String> → IntStream
intStream.mapToObj(i -> "N" + i)        // IntStream → Stream<String>
intStream.boxed()                        // IntStream → Stream<Integer>
```

---

## Полная схема конвейера

```
        ИСТОЧНИК              ПРОМЕЖУТОЧНЫЕ (lazy)         ТЕРМИНАЛЬНЫЙ (eager)
┌─────────────────────┐    ┌──────────────────────────┐    ┌──────────────────┐
│ collection.stream() │ →  │ .filter(Predicate)        │ → │ .collect(...)    │
│ Arrays.stream(arr)  │    │ .map(Function)            │    │ .forEach(...)    │
│ Stream.of(...)      │    │ .flatMap(Function)        │    │ .count()         │
│ Stream.generate()   │    │ .sorted(Comparator)       │    │ .reduce(...)     │
│ IntStream.range()   │    │ .distinct()               │    │ .findFirst()     │
└─────────────────────┘    │ .limit(n)                 │    │ .anyMatch(...)   │
                           │ .skip(n)                  │    │ .min() / .max()  │
                           │ .peek(Consumer)           │    └──────────────────┘
                           └──────────────────────────┘
                              ↑ не выполняются, пока не вызван терминальный метод
```
