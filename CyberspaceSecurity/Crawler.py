# Crawler.py
import requests# 导入所需库
from bs4 import BeautifulSoup# 导入BeautifulSoup用于HTML解析
import time# 导入time用于延迟
import re# 导入正则表达式库
import logging# 导入日志库
import csv# 导入csv库用于保存数据
from typing import List, Dict, Optional# 导入类型提示

# 设置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)# 创建日志记录器

# 定义爬虫类
class BookScraper:
    def __init__(self, base_url: str = "http://books.toscrape.com", delay: float = 1.0):# 初始化爬虫
        self.base_url = base_url# 基础URL
        self.delay = delay# 请求间隔时间
        self.session = requests.Session()# 使用Session保持连接
        # 设置请求头，模拟浏览器行为
        self.session.headers.update({
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
        })
        self.books_data = []# 存储爬取的图书数据

    # 获取页面内容    
    def get_page_content(self, url: str) -> Optional[BeautifulSoup]:
        try:# 发送GET请求
            response = self.session.get(url, timeout=10)
            response.raise_for_status()
            return BeautifulSoup(response.content, 'html.parser')
        except requests.RequestException as e:# 处理请求异常
            logger.error(f"请求失败 {url}: {e}")
            return None
    
    # 获取前max_pages页的图书链接
    def get_limited_page_links(self, max_pages: int = 10) -> List[str]:
        """获取前max_pages页的图书链接"""
        book_links = []
        # 遍历每一页
        for page_num in range(1, max_pages + 1):
            # 构造页面URL
            if page_num == 1:
                url = f"{self.base_url}/index.html"
            else:
                url = f"{self.base_url}/catalogue/page-{page_num}.html"

            # 获取页面内容
            logger.info(f"正在爬取第 {page_num}/{max_pages} 页: {url}")
            soup = self.get_page_content(url)

            # 检查页面是否获取成功
            if not soup:
                logger.warning(f"无法获取第 {page_num} 页内容，跳过")
                continue
            
            # 查找图书链接
            books = soup.find_all('article', class_='product_pod')
            logger.info(f"在第 {page_num} 页找到 {len(books)} 本图书")
            
            # 提取每本图书的链接
            for book in books:
                link = book.find('h3').find('a')['href']
                # 处理相对URL
                if link.startswith('../../../'):
                    full_link = link.replace('../../../', f'{self.base_url}/')
                else:
                    full_link = f"{self.base_url}/catalogue/{link}"
                book_links.append(full_link)
            
            # 检查是否还有更多页面（提前停止）
            if page_num < max_pages:
                next_button = soup.find('li', class_='next')
                if not next_button:
                    logger.info(f"第 {page_num} 页后没有更多页面，提前结束")
                    break
            
            # 页面间延迟
            time.sleep(self.delay)

        # 输出总链接数    
        logger.info(f"共找到 {len(book_links)} 本图书链接")
        return book_links
    
    # 解析图书详情
    def parse_book_details(self, url: str) -> Optional[Dict]:
        logger.info(f"正在解析图书详情: {url}")
        soup = self.get_page_content(url)
        
        # 检查页面是否获取成功
        if not soup:
            return None
        
        # 解析图书信息    
        try:
            book_info = {}
            
            # 书名
            title = soup.find('div', class_='product_main').find('h1')
            book_info['Title'] = title.text.strip() if title else "N/A"
            
            # 价格
            price = soup.find('p', class_='price_color')
            if price:
                price_text = price.text.strip()
                price_match = re.search(r'[\d.]+', price_text)
                book_info['Price'] = float(price_match.group()) if price_match else 0.0
            else:
                book_info['Price'] = 0.0
            
            # 库存状态
            availability = soup.find('p', class_='instock')
            if availability:
                availability_text = availability.text.strip()
                stock_match = re.search(r'\d+', availability_text)
                book_info['Availability'] = int(stock_match.group()) if stock_match else 0
            else:
                book_info['Availability'] = 0
            
            # 图书分类
            breadcrumb = soup.find('ul', class_='breadcrumb')
            if breadcrumb:
                category_link = breadcrumb.find_all('li')[-2].find('a')
                book_info['Category'] = category_link.text.strip() if category_link else "N/A"
            else:
                book_info['Category'] = "N/A"
            
            # 产品描述
            description = soup.find('div', id='product_description')
            if description:
                description_text = description.find_next_sibling('p')
                book_info['Description'] = description_text.text.strip() if description_text else "N/A"
            else:
                book_info['Description'] = "N/A"
            
            # 评价星级
            star_rating = soup.find('p', class_='star-rating')
            if star_rating:
                rating_classes = star_rating.get('class', [])
                rating_map = {'One': 1, 'Two': 2, 'Three': 3, 'Four': 4, 'Five': 5}
                for cls in rating_classes:
                    if cls in rating_map:
                        book_info['Rating'] = rating_map[cls]
                        break
                else:
                    book_info['Rating'] = 0
            else:
                book_info['Rating'] = 0
            
            # 产品编码
            table = soup.find('table', class_='table table-striped')
            if table:
                rows = table.find_all('tr')
                for row in rows:
                    th = row.find('th')
                    if th and th.text.strip() == 'UPC':
                        td = row.find('td')
                        book_info['UPC'] = td.text.strip() if td else "N/A"
                        break
                else:
                    book_info['UPC'] = "N/A"
            else:
                book_info['UPC'] = "N/A"
            
            logger.info(f"成功解析: {book_info['Title']} - £{book_info['Price']:.2f}")
            return book_info
            
        except Exception as e:
            logger.error(f"解析图书详情失败 {url}: {e}")
            return None
    
    # 爬取图书数据
    def scrape_books(self, max_pages: int = 10) -> List[Dict]:
        """爬取前max_pages页的图书"""
        logger.info(f"开始爬取前 {max_pages} 页图书...")
        book_links = self.get_limited_page_links(max_pages)
        # 逐本处理图书链接
        total_books = len(book_links)
        success_count = 0
        
        # 遍历每本图书链接
        for i, link in enumerate(book_links, 1):
            logger.info(f"正在处理第 {i}/{total_books} 本图书")
            book_info = self.parse_book_details(link)
            # 记录成功的图书信息
            if book_info:
                self.books_data.append(book_info)
                success_count += 1
            else:
                logger.warning(f"第 {i} 本图书解析失败")
            
            # 添加延迟，避免请求过快
            time.sleep(self.delay)
            
            # 每10本显示一次进度
            if i % 10 == 0:
                logger.info(f"进度: {i}/{total_books} ({success_count} 本成功)")
        
        logger.info(f"爬取完成！成功获取 {success_count}/{total_books} 本图书信息")
        return self.books_data
    
    # 保存数据到CSV文件
    def save_to_csv(self, filename: str = "books_data.csv"):
        if not self.books_data:
            logger.warning("没有数据可保存")
            return
            
        # 使用csv模块保存数据
        with open(filename, 'w', newline='', encoding='utf-8') as csvfile:
            fieldnames = ['Title', 'Price', 'Availability', 'Category', 'Description', 'Rating', 'UPC']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            
            # 写入表头
            writer.writeheader()
            for book in self.books_data:
                writer.writerow(book)
        
        logger.info(f"数据已保存到 {filename}")
        print(f"\n生成的CSV文件包含以下列：{', '.join(fieldnames)}")
        print(f"共保存了 {len(self.books_data)} 本图书信息")
    
    # 分析数据
    def analyze_data(self):
        """直接分析内存中的数据"""
        if not self.books_data:
            print("没有数据可供分析")
            return
        
        # 统计信息输出
        print("\n" + "="*50)
        print("数据分析结果")
        print("="*50)
        
        # 最贵的5本书
        print("\n1. 最贵的5本书:")
        sorted_books = sorted(self.books_data, key=lambda x: x['Price'], reverse=True)
        for i in range(min(5, len(sorted_books))):
            book = sorted_books[i]
            print(f"   {book['Title']} - £{book['Price']:.2f}")
        
        # 图书数量最多的前3个分类
        print("\n2. 图书数量最多的前3个分类:")
        category_count = {}
        for book in self.books_data:
            category = book['Category']
            category_count[category] = category_count.get(category, 0) + 1
        
        # 按数量排序并输出前3个分类
        sorted_categories = sorted(category_count.items(), key=lambda x: x[1], reverse=True)
        for i in range(min(3, len(sorted_categories))):
            category, count = sorted_categories[i]
            print(f"   {category}: {count} 本")
        
        # 所有图书的平均价格
        avg_price = sum(book['Price'] for book in self.books_data) / len(self.books_data)
        print(f"\n3. 所有图书的平均价格: £{avg_price:.2f}")
        
        # 额外统计：星级分布
        print(f"\n4. 星级分布:")
        rating_count = {}
        for book in self.books_data:
            rating = book['Rating']
            rating_count[rating] = rating_count.get(rating, 0) + 1
        
        for rating in sorted(rating_count.keys()):
            count = rating_count[rating]
            stars = "★" * rating
            print(f"   {rating}星 {stars}: {count} 本")
        
        # 额外统计：价格范围
        prices = [book['Price'] for book in self.books_data]
        print(f"\n5. 价格统计:")
        print(f"   最高价格: £{max(prices):.2f}")
        print(f"   最低价格: £{min(prices):.2f}")
        print(f"   价格中位数: £{sorted(prices)[len(prices)//2]:.2f}")

def main():
    # 创建爬虫实例，设置较短的延迟
    scraper = BookScraper(delay=0.3)  # 减少延迟以加快速度
    
    print("开始爬取图书数据...")
    print("目标网站: http://books.toscrape.com")
    print("爬取前10页图书数据\n")
    
    # 爬取前10页图书
    books_data = scraper.scrape_books(max_pages=10)
    
    if books_data:
        # 保存到CSV
        scraper.save_to_csv("books_data.csv")
        
        # 数据分析
        scraper.analyze_data()
        
        print(f"\n爬取完成！共获取 {len(books_data)} 本图书信息")
        print("数据已保存到: books_data.csv")
        
        # 显示一些统计信息
        categories = set(book['Category'] for book in books_data)
        print(f"涉及分类数量: {len(categories)}")
        print(f"数据总量: {len(books_data)} 条记录")
    else:
        print("爬取失败，没有获取到任何数据")

if __name__ == "__main__":
    main()