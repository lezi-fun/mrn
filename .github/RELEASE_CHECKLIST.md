# Release Checklist

## Pre-Release
- [ ] 确保所有测试通过
- [ ] 更新版本号
- [ ] 更新 CHANGELOG.md
- [ ] 验证跨平台构建
- [ ] 运行性能测试

## Release Process
1. 创建并推送标签: `git tag v1.0.0 && git push origin v1.0.0`
2. GitHub Action 会自动运行构建和发布流程
3. 验证发布的二进制文件
4. 更新文档和网站（如果有）

## Post-Release
- [ ] 公告发布
- [ ] 更新包管理器（未来支持）
- [ ] 监控问题反馈
